#ifndef KOIOS_TASK_H
#define KOIOS_TASK_H

#include <utility>
#include <memory>
#include <source_location>
#include <mutex>
#include <concepts>
#include <iostream>

#include "fmt/format.h"

#include "koios/macros.h"
#include "koios/promise_base.h"
#include "koios/promise_wrapper.h"
#include "koios/return_value_or_void.h"
#include "koios/task_scheduler_wrapper.h"
#include "koios/get_result_aw.h"
#include "koios/driver_policy.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

/*! \brief Wrapper class of task.
 *  Prevent ADL lookup.
 *
 *  \tparam T The return value type just like a regular function.
 *  \tparam DriverPolicy One of `run_this_async` or `run_this_sync`.
 *  \tparam Discardable One of `discardable` or `non_discardable`.
 */
template<typename, driver_policy_concept, typename>
struct _task
{
    struct [[nodiscard]] _type;
};

struct discardable{};
struct non_discardable{};

/*! \brief The most basic unit of the runtime.
 *
 *  All of the user's asynchronous tasks managed by koios are represented by this class. 
 *  This is a class backed by the C++20 coroutine facility.
 *  This class is also one of the `std::coroutine_handle` ownership phase manager.
 *  If you just call a coroutine without `co_await` or `task::run()` or etc.
 *  This type of object will holds this ownership of the handler. 
 *  Once scheduled this ownership will be taken by `task_scheduler`, 
 *  and more precisely the class `task_on_the_fly`.
 *
 *  \see `task_on_the_fly`
 */
template<
    typename T, 
    driver_policy_concept DriverPolicy,
    typename Discardable>
class _task<T, DriverPolicy, Discardable>::_type 
    : public get_result_aw<T, _task<T, DriverPolicy, Discardable>::_type, DriverPolicy>
{
public:
    using value_type = T;
    using future_type = ::std::future<value_type>;

    class promise_type 
        : public promise_base<destroy_aw>, 
          public return_value_or_void<T, promise_type, DriverPolicy>
    {
    public:
        _task<T, DriverPolicy, Discardable>::_type get_return_object() noexcept
        {
            return { *this };
        }

        void unhandled_exception()
        {
            return_value_or_void<T, promise_type, DriverPolicy>::deal_exception(::std::current_exception());
        }
    };

    friend class task_scheduler;

    template<typename, typename, typename>
    friend class get_result_aw_base;

private:
    _type(promise_type& p)
        : get_result_aw<T, _type, DriverPolicy>(p),
          m_coro_handle{ ::std::coroutine_handle<promise_type>::from_promise(p) }, 
          m_std_promise_p{ p.get_std_promise_pointer() }
    {
    }

public:
    /*! Of course move constructor will move the ownership of the handler. */
    _type(_type&& other) noexcept
        : get_result_aw<T, _task<T, DriverPolicy, Discardable>::_type, DriverPolicy>(::std::move(other)),
          m_coro_handle{ ::std::move(other.m_coro_handle) }
    {
    }

    operator task_on_the_fly() noexcept
    {
        return get_handler_to_schedule();
    }

    /*! \brief Run the current task.
     *  Just call the `run()` member function.
     *  \see `run()`
     *  \see `run_and_get_future()`
     */
    void operator()() 
    {
        run();
    }

    /*! \retval true The coroutine state was suspended at final state, 
     *          or the task has been executed by `run()` or other similar functions.
     *  \retval false The coroutine state was suspended at state which is NOT final.
     */
    bool done() const noexcept 
    {
        return m_coro_handle.done();
    }

    /*! \brief Run the task.
     *  
     *  Run the task on a scheduler which specified by the `DriverPolicy`.
     *  If you call this function with a `nodiscard_task`,
     *  the static_assert will stop the compiling.
     *  
     *  \see `run_and_get_future()`
     */
    void run()
    {
        static_assert(is_discardable(), 
                      "This is an non-discardable task, "
                      "you should call `run_and_get_future()` nor `run()`.");
        DriverPolicy{}.scheduler().enqueue(get_handler_to_schedule());
    }

    /*! \brief Run the task.
     *  
     *  Similar to `run()`
     *  \return The related future object.
     *  
     *  \see `run()`
     *  \see `get_future()`
     */
    [[nodiscard]] future_type run_and_get_future()
    {
        auto result = get_future();
        if (!has_scheduled())
        {   
            DriverPolicy{}.scheduler().enqueue(get_handler_to_schedule());
        }
        return result;
    }

    [[nodiscard]] auto result()
    {
        if constexpr (is_return_void())
            run_and_get_future().get();
        else return run_and_get_future().get();
    }

    /*! \retval true This task is a discardable task. You could ignore the return value.
     *  \retval false This task is NOT a Discardable task. You have to take the ownership of the related future object.
     *
     *  And this is a static consteval function.
     */
    [[nodiscard]] static consteval bool is_discardable() 
    {
        return ::std::same_as<Discardable, discardable>;
    }

private:
    [[nodiscard]] bool has_got_future() const noexcept { return bool(m_std_promise_p); }
    [[nodiscard]] bool has_scheduled() const noexcept { return !m_coro_handle; }
    [[nodiscard]] static consteval bool is_return_void() { return ::std::same_as<void, value_type>; }

    /*! \brief Take the ownership of the future object related to this task.
     *  \return the future object.
     *  \see `std::future`.
     *
     *  \warning You can only call this fuction only once BEFORE `get_future()` and `run()`.
     *           Or you will get a `std::logic_error`.
     */
    [[nodiscard]] future_type get_future()
    {
        if (has_scheduled())
            throw ::std::logic_error{ "You should call `get_future()` before `run()`" };

        return get_result_aw<T, _type, DriverPolicy>::get_future();
    }

    auto get_handler_to_schedule() noexcept { return ::std::exchange(m_coro_handle, {}); }

private:
    task_on_the_fly m_coro_handle;
    ::std::shared_ptr<::std::promise<value_type>> m_std_promise_p{};
};

template<typename T>
using async_task = typename _task<T, run_this_async, discardable>::_type;

template<typename T>
using sync_task = typename _task<T, run_this_sync, discardable>::_type;

template<typename T>
using nodiscard_task = typename _task<T, run_this_async, non_discardable>::_type;

template<typename T>
using task = async_task<T>;

KOIOS_NAMESPACE_END

#endif
