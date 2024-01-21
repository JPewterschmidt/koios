/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
#include "koios/future.h"
#include "koios/per_consumer_attr.h"

KOIOS_NAMESPACE_BEG

/*! \brief Wrapper class of task.
 *  Prevent ADL lookup.
 *
 *  \tparam T The return value type just like a regular function.
 *  \tparam DriverPolicy One of `run_this_async` or `run_this_sync`.
 *  \tparam Discardable One of `discardable` or `non_discardable`.
 */
template<typename, driver_policy_concept, typename, typename>
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
    typename Discardable, 
    typename InitialSuspendAw>
class _task<T, DriverPolicy, Discardable, InitialSuspendAw>::_type 
    : public get_result_aw<T, _task<T, DriverPolicy, Discardable, InitialSuspendAw>::_type, DriverPolicy>
{
public:
    using value_type = T;
    using future_type = koios::future<value_type>;
    using initial_suspend_type = InitialSuspendAw;

    class promise_type 
        : public promise_base<InitialSuspendAw, destroy_aw>, 
          public return_value_or_void<T, promise_type, DriverPolicy>
    {
    public:
        _task<T, DriverPolicy, Discardable, initial_suspend_type>::_type 
        get_return_object() noexcept
        {
            return { *this };
        }

        void unhandled_exception()
        {
            this->deal_exception(::std::current_exception());
        }
    };

    friend class task_scheduler;

    template<typename, typename, typename>
    friend class get_result_aw_base;

protected:
    _type(promise_type& p)
        : get_result_aw<T, _type, DriverPolicy>(p),
          m_coro_handle{ ::std::coroutine_handle<promise_type>::from_promise(p) }, 
          m_std_promise_p{ p.get_std_promise_pointer() }
    {
        if (is_eager()) m_coro_handle.give_up_ownership();
    }

public:
    /*! Of course move constructor will move the ownership of the handler. */
    _type(_type&& other) noexcept
        : get_result_aw<T, _task<T, DriverPolicy, Discardable, initial_suspend_type>::_type, DriverPolicy>(::std::move(other)),
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
        auto schr = DriverPolicy{}.scheduler();
        run_on(schr);
    }

    void run(const per_consumer_attr& attr)
    {
        auto schr = DriverPolicy{}.scheduler();
        run_on(attr, schr);
    }

    void 
    run_on(const task_scheduler_wrapper& schr)
    {
        static_assert(is_return_void() || is_discardable(), 
                      "This is an non-discardable task, "
                      "you should call `run_and_get_future()` nor `run()`.");
        if (!is_eager() && !this->future().ready())
            schr.enqueue(get_handler_to_schedule());
    }

    void 
    run_on(const per_consumer_attr& attr, const task_scheduler_wrapper& schr)
    {
        static_assert(is_return_void() || is_discardable(), 
                      "This is an non-discardable task, "
                      "you should call `run_and_get_future()` nor `run()`.");
        if (!is_eager() && !this->future().ready())
            schr.enqueue(attr, get_handler_to_schedule());
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
        auto schr = DriverPolicy{}.scheduler();
        return run_and_get_future_on(schr);
    }

    [[nodiscard]] future_type run_and_get_future(const per_consumer_attr& attr)
    {
        auto schr = DriverPolicy{}.scheduler();
        return run_and_get_future_on(attr, schr);
    }

    [[nodiscard]] future_type run_and_get_future_on(const task_scheduler_wrapper& schr)
    {
        auto result = get_future();
        if (!has_scheduled() && !is_eager() && !result.ready())
        {   
            schr.enqueue(get_handler_to_schedule());
        }
        return result;
    }

    [[nodiscard]] future_type run_and_get_future_on(
        const per_consumer_attr& attr, const task_scheduler_wrapper& schr)
    {
        auto result = get_future();
        if (!has_scheduled() && !is_eager() && !result.ready())
        {   
            schr.enqueue(attr, get_handler_to_schedule());
        }
        return result;
    }

    [[nodiscard]] auto result()
    {
        auto schr = DriverPolicy{}.scheduler();
        return result_on(schr);
    }

    [[nodiscard]] auto result(const per_consumer_attr& attr)
    {
        auto schr = DriverPolicy{}.scheduler();
        return result_on(attr, schr);
    }

    [[nodiscard]] auto result_on(const task_scheduler_wrapper& schr)
    {
        if constexpr (is_return_void())
            run_and_get_future_on(schr).get();
        else return run_and_get_future_on(schr).get();
    }

    [[nodiscard]] auto result_on(const per_consumer_attr& attr, const task_scheduler_wrapper& schr)
    {
        if constexpr (is_return_void())
            run_and_get_future_on(attr, schr).get();
        else return run_and_get_future_on(attr, schr).get();
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

    [[nodiscard]] static consteval bool is_eager()
    {
        return ::std::same_as<initial_suspend_type, eager_aw>;
    }

private:
    [[nodiscard]] bool has_got_future() const noexcept { return bool(m_std_promise_p); }
    [[nodiscard]] static consteval bool is_return_void() { return ::std::same_as<void, value_type>; }

    [[nodiscard]] bool has_scheduled() const noexcept 
    { 
        return !m_coro_handle; 
    }

    /*! \brief Take the ownership of the future object related to this task.
     *  \return the future object.
     *  \see `std::future`.
     *
     *  \warning You can only call this fuction only once BEFORE `get_future()` and `run()`.
     *           Or you will get a `std::logic_error`.
     */
    [[nodiscard]] future_type get_future()
    {
        if (!is_eager() && has_scheduled())
            throw ::std::logic_error{ "You should call `get_future()` before `run()`" };

        return get_result_aw<T, _type, DriverPolicy>::get_future();
    }

    auto get_handler_to_schedule() noexcept { return ::std::exchange(m_coro_handle, {}); }

private:
    task_on_the_fly m_coro_handle;
    ::std::shared_ptr<koios::promise<value_type>> m_std_promise_p{};
};

template<typename T = void, typename InitialSuspendAw = eager_aw>
using async_task = typename _task<T, run_this_async, discardable, InitialSuspendAw>::_type;

template<typename T = void, typename InitialSuspendAw = eager_aw>
using nodiscard_task = typename _task<T, run_this_async, non_discardable, InitialSuspendAw>::_type;

template<typename T = void, typename InitialSuspendAw = eager_aw>
using task = async_task<T, InitialSuspendAw>;

template<typename T = void, typename InitialSuspendAw = ::std::suspend_always>
using emitter_task = async_task<T, InitialSuspendAw>;

KOIOS_NAMESPACE_END

#endif
