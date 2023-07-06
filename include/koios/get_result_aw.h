#ifndef KOIOS_GET_RESULT_AW_H
#define KOIOS_GET_RESULT_AW_H

#include <future>

#include "koios/macros.h"
#include "koios/task_scheduler_selector.h"
#include "koios/local_thread_scheduler.h"
#include "koios/task_scheduler_wrapper.h"

KOIOS_NAMESPACE_BEG

template<typename T, typename Task>
class get_result_aw 
    : public virtual task_scheduler_selector
{
public:
    using value_type = T;

    get_result_aw(promise_wrapper<value_type> promise)
        : m_future{ promise.get_future() }, 
          m_promise{ ::std::move(promise) }
    {
    }
    
    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(::std::coroutine_handle<> h)
    {
        m_promise.set_caller(h);
        scheduler().enqueue(
            static_cast<Task*>(this)->move_out_coro_handle()
        );
    }

    decltype(auto) await_resume() noexcept { return m_future.get(); }

    auto& future() noexcept { return m_future; }

protected:
    ::std::future<T> m_future;
    promise_wrapper<value_type> m_promise;
};

template<typename Task>
class get_result_aw<void, Task>
    : public virtual task_scheduler_selector
{
public:
    get_result_aw(promise_wrapper<void> promise)
        : m_future{ promise.get_future() }, 
          m_promise{ ::std::move(promise) }
    {
    }

    constexpr bool await_ready() const { return false; }
    void await_suspend(::std::coroutine_handle<> h)
    {
        m_promise.set_caller(h);
        scheduler().enqueue(
            static_cast<Task*>(this)->move_out_coro_handle()
        );
    }

    constexpr void await_resume() const noexcept {}

    auto& future() noexcept { return m_future; }

protected:
    ::std::future<void> m_future;
    promise_wrapper<void> m_promise;
};

KOIOS_NAMESPACE_END

#endif
