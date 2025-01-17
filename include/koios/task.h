// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_TASK_H
#define KOIOS_TASK_H

#include <utility>
#include <memory>
#include <source_location>
#include <mutex>
#include <concepts>
#include <string>
#include <string_view>
#include <span>
#include <cstddef>

#include "koios/macros.h"
#include "koios/promise_base.h"
#include "koios/promise_wrapper.h"
#include "koios/return_value_or_void.h"
#include "koios/task_scheduler.h"
#include "koios/task_on_the_fly.h"
#include "koios/future.h"
#include "koios/per_consumer_attr.h"

KOIOS_NAMESPACE_BEG

/*! \brief Wrapper class of task.
 *  Prevent ADL lookup.
 *
 *  \tparam T The return value type just like a regular function.
 *  \tparam Discardable One of `discardable` or `non_discardable`.
 */
template<typename, typename, typename>
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
    typename Discardable, 
    typename InitialSuspendAw>
class _task<T, Discardable, InitialSuspendAw>::_type 
{
public:
    using value_type = T;
    using future_type = koios::future<value_type>;
    using initial_suspend_type = InitialSuspendAw;

    // This should be always a move only object, 
    // `return_value_or_void` was derived from `toolpex::move_only`
    // See blog https://devblogs.microsoft.com/oldnewthing/20210504-00/?p=105176
    // by Raymond Chen
    class promise_type 
        : public promise_base<InitialSuspendAw, destroy_aw>, 
          public return_value_or_void<T, promise_type>
    {
    public:
        _task<T, Discardable, initial_suspend_type>::_type 
        get_return_object() noexcept { return { *this }; }
        void unhandled_exception() { this->deal_exception(::std::current_exception()); }
    };

    friend class task_scheduler;

protected:
    _type(promise_type& p)
        : m_coro_handle{ ::std::coroutine_handle<promise_type>::from_promise(p) }, 
          m_future{ p.get_future() }
    {
        if constexpr (this->is_eager()) m_coro_handle.give_up_ownership();
    }

public:
    /*! Of course move constructor will move the ownership of the handler. */
    _type(_type&& other) noexcept
        : m_coro_handle{ ::std::move(other.m_coro_handle) }, 
          m_future{ ::std::move(other.m_future) }
    {
    }

    operator task_on_the_fly() noexcept { return this->get_handler_to_schedule(); }

    /*! \brief Run the current task.
     *  Just call the `run()` member function.
     *  \see `run()`
     *  \see `run_and_get_future()`
     */
    void operator()() { this->run(); }

    /*! \retval true The coroutine state was suspended at final state, 
     *          or the task has been executed by `run()` or other similar functions.
     *  \retval false The coroutine state was suspended at state which is NOT final.
     */
    bool done() const noexcept { return m_coro_handle.done(); }

    /*! \brief Run the task.
     *  
     *  Run the task on the scheduler.
     *  If you call this function with a `nodiscard_task`,
     *  the static_assert will stop the compiling.
     *  
     *  \see `run_and_get_future()`
     */
    void run()
    {
        this->run_on(get_task_scheduler());
    }

    void run(const per_consumer_attr& attr)
    {
        this->run_on(attr, get_task_scheduler());
    }

    void 
    run_on(task_scheduler& schr)
    {
        static_assert(this->is_return_void() || this->is_discardable(), 
                      "This is an non-discardable task, "
                      "you should call `run_and_get_future()` nor `run()`.");
        if constexpr (!this->is_eager())
        {
            if (!m_future.ready())
            {   
                auto h = this->get_handler_to_schedule();
                [[assume(bool(h))]];
                schr.enqueue(::std::move(h));
            }
        }
    }

    void 
    run_on(const per_consumer_attr& attr, task_scheduler& schr)
    {
        static_assert(this->is_return_void() || this->is_discardable(), 
                      "This is an non-discardable task, "
                      "you should call `run_and_get_future()` nor `run()`.");
        if constexpr (!this->is_eager())
        {
            if (!m_future.ready())
            {   
                auto h = this->get_handler_to_schedule();
                [[assume(bool(h))]];
                schr.enqueue(attr, ::std::move(h));
            }
        }
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
        return this->run_and_get_future_on(get_task_scheduler());
    }

    [[nodiscard]] future_type run_and_get_future(const per_consumer_attr& attr)
    {
        return this->run_and_get_future_on(attr, get_task_scheduler());
    }

    [[nodiscard]] future_type run_and_get_future_on(task_scheduler& schr)
    {
        auto result = this->get_future();
        if constexpr (!this->is_eager())
        {
            if (!this->has_scheduled() && !result.ready())
            {   
                auto h = this->get_handler_to_schedule();
                [[assume(bool(h))]];
                schr.enqueue(::std::move(h));
            }
        }
        return result;
    }

    [[nodiscard]] future_type run_and_get_future_on(
        const per_consumer_attr& attr, task_scheduler& schr)
    {
        auto result = this->get_future();
        if constexpr (!this->is_eager())
        {
            if (!this->has_scheduled() && !result.ready())
            {   
                auto h = this->get_handler_to_schedule();
                [[assume(bool(h))]];
                schr.enqueue(attr, ::std::move(h));
            }
        }
        return result;
    }

    auto operator co_await()
    {
        if (!m_future.valid())
        {
            throw ::std::logic_error{ "task::operator co_await(): you have already called task::get_future()." };
        }
        get_task_scheduler().enqueue(this->get_handler_to_schedule());
        return m_future.get_async();
    }

    [[nodiscard]] auto result()
    {
        return this->result_on(get_task_scheduler());
    }

    [[nodiscard]] auto result(const per_consumer_attr& attr)
    {
        return this->result_on(attr, get_task_scheduler());
    }

    [[nodiscard]] auto result_on(task_scheduler& schr)
    {
        if constexpr (this->is_return_void())
            this->run_and_get_future_on(schr).get();
        else return this->run_and_get_future_on(schr).get();
    }

    [[nodiscard]] auto result_on(const per_consumer_attr& attr, task_scheduler& schr)
    {
        if constexpr (this->is_return_void())
            this->run_and_get_future_on(attr, schr).get();
        else return this->run_and_get_future_on(attr, schr).get();
    }

    /*! \retval true This task is a discardable task. You could ignore the return value.
     *  \retval false This task is NOT a Discardable task. You have to take the ownership of the related future object.
     *
     *  And this is a static consteval function.
     */
    [[nodiscard]] static consteval bool is_discardable() { return ::std::same_as<Discardable, discardable>; }
    [[nodiscard]] static consteval bool is_eager() { return ::std::same_as<initial_suspend_type, eager_aw>; }

private:
    [[nodiscard]] static consteval bool is_return_void() { return ::std::same_as<void, value_type>; }
    [[nodiscard]] bool has_scheduled() const noexcept { return !m_coro_handle; }

    /*! \brief Take the ownership of the future object related to this task.
     *  \return the future object.
     *  \see `std::future`.
     *
     *  \warning You can only call this fuction only once BEFORE `get_future()` and `run()`.
     *           Or you will get a `std::logic_error`.
     */
    [[nodiscard]] future_type get_future()
    {
        if constexpr (!this->is_eager())
        {
            if (this->has_scheduled()) throw ::std::logic_error{ "You should call `get_future()` before `run()`" };
        }

        return ::std::move(m_future);
    }

    auto get_handler_to_schedule() noexcept { return ::std::exchange(m_coro_handle, {}); }

private:
    task_on_the_fly m_coro_handle;
    future_type m_future;
};

template<typename T = void, typename InitialSuspendAw = eager_aw>
using async_task = typename _task<T, discardable, InitialSuspendAw>::_type;

template<typename T = void, typename InitialSuspendAw = eager_aw>
using nodiscard_task = typename _task<T, non_discardable, InitialSuspendAw>::_type;

template<typename T = void, typename InitialSuspendAw = eager_aw>
using task = async_task<T, InitialSuspendAw>;

template<typename T = void, typename InitialSuspendAw = lazy_aw>
using lazy_task = async_task<T, InitialSuspendAw>;

using taskec = task<::std::error_code>;
using lztaskec = lazy_task<::std::error_code>;

extern template class koios::_task<void, discardable, ::std::suspend_always>::_type;
extern template class koios::_task<void, discardable, eager_aw>::_type;
extern template class koios::_task<void, non_discardable, eager_aw>::_type;
extern template class koios::_task<bool, discardable, eager_aw>::_type;
extern template class koios::_task<int, discardable, eager_aw>::_type;
extern template class koios::_task<size_t, discardable, eager_aw>::_type;
extern template class koios::_task<::std::string, discardable, eager_aw>::_type;
extern template class koios::_task<::std::string_view, discardable, eager_aw>::_type;
extern template class koios::_task<::std::error_code, discardable, eager_aw>::_type;
extern template class koios::_task<uint8_t, discardable, eager_aw>::_type;
extern template class koios::_task<uint32_t, discardable, eager_aw>::_type;
extern template class koios::_task<::std::byte*, discardable, eager_aw>::_type;
extern template class koios::_task<const ::std::byte*, discardable, eager_aw>::_type;
extern template class koios::_task<char*, discardable, eager_aw>::_type;
extern template class koios::_task<const char*, discardable, eager_aw>::_type;
extern template class koios::_task<void*, discardable, eager_aw>::_type;
extern template class koios::_task<const void*, discardable, eager_aw>::_type;
extern template class koios::_task<::std::span<::std::byte>, discardable, eager_aw>::_type;
extern template class koios::_task<::std::span<const ::std::byte>, discardable, eager_aw>::_type;
extern template class koios::_task<::std::span<char>, discardable, eager_aw>::_type;
extern template class koios::_task<::std::span<const char>, discardable, eager_aw>::_type;
extern template class koios::_task<::std::span<uint8_t>, discardable, eager_aw>::_type;
extern template class koios::_task<::std::span<const uint8_t>, discardable, eager_aw>::_type;

KOIOS_NAMESPACE_END

#endif
