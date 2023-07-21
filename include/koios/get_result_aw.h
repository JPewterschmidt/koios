#ifndef KOIOS_GET_RESULT_AW_H
#define KOIOS_GET_RESULT_AW_H

#include <future>
#include <memory>
#include <cassert>

#include "koios/macros.h"
#include "koios/local_thread_scheduler.h"
#include "koios/task_scheduler_wrapper.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

template<typename T, typename Task, typename DriverPolicy>
class get_result_aw_base
{
public:
    using value_type = T;

    get_result_aw_base(promise_wrapper<value_type> p)
        : m_promise{ ::std::move(p) }, 
          m_future{ m_promise.get_future() }
    {
    }

    constexpr bool await_ready() const noexcept { return false; }
    //void await_suspend(::std::coroutine_handle<> h)
    void await_suspend(task_on_the_fly h)
    {
        m_promise.set_caller(::std::move(h));
        DriverPolicy{}.scheduler().enqueue(
            static_cast<Task*>(this)->get_handler_to_schedule()
        );
    }

    auto get_future() noexcept 
    { 
        // For which scheduled by user call `task::run()` or `task::run_and_get_future()` directly.
        assert(!m_promise.caller_set());
        return ::std::move(m_future); 
    }

protected:
    promise_wrapper<value_type> m_promise;
    ::std::future<T> m_future;
};

template<typename T, typename Task, typename DriverPolicy>
class get_result_aw : public get_result_aw_base<T, Task, DriverPolicy>
{
public:
    using value_type = T;

    get_result_aw(promise_wrapper<value_type> promise)
        : get_result_aw_base<T, Task, DriverPolicy>{ ::std::move(promise) }
    {
    }
    
    decltype(auto) await_resume() noexcept { return get_result_aw_base<T, Task, DriverPolicy>::m_future.get(); }
};

template<typename Task, typename DriverPolicy>
class get_result_aw<void, Task, DriverPolicy> : public get_result_aw_base<void, Task, DriverPolicy>
{
public:
    get_result_aw(promise_wrapper<void> promise)
        : get_result_aw_base<void, Task, DriverPolicy>{ ::std::move(promise) }
    {
    }

    constexpr void await_resume() const noexcept {}
};

KOIOS_NAMESPACE_END

#endif
