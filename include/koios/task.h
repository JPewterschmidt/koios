#ifndef KOIOS_TASK_H
#define KOIOS_TASK_H

#include <utility>

#include "koios/macros.h"
#include "koios/promise_base.h"
#include "koios/global_task_scheduler.h"
#include "koios/promise_wrapper.h"
#include "koios/task_scheduler_concept.h"

KOIOS_NAMESPACE_BEG

template<typename T>
struct _task
{
    struct [[nodiscard]] _type;
};

template<typename T, typename Task>
class get_result_aw
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
        auto* taskp = static_cast<Task*>(this);
        task_scheduler_concept auto& schr = get_task_scheduler();
        schr.enqueue(taskp->move_out_coro_handle());
    }

    auto await_resume() noexcept { return m_future.get(); }

    auto& future() noexcept { return m_future; }

protected:
    ::std::future<T> m_future;
    promise_wrapper<value_type> m_promise;
};

template<typename Task>
class get_result_aw<void, Task>
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
        auto* taskp = static_cast<Task*>(this);
        task_scheduler_concept auto& schr = get_task_scheduler();
        schr.enqueue(taskp->move_out_coro_handle());
    }

    constexpr void await_resume() const noexcept {}

    auto& future() noexcept { return m_future; }

protected:
    ::std::future<void> m_future;
    promise_wrapper<void> m_promise;
};

template<typename T>
class _task<T>::_type : public get_result_aw<T, _task<T>::_type>
{
public:
    using value_type = T;

    class promise_type : public promise_base
    {
    public:
        _task<T>::_type get_return_object() noexcept
        {
            return { *this };
        }

        auto get_future() { return m_promise.get_future(); }
        void set_caller(auto h) { m_caller = h; }
        
        void unhandled_exception() const { throw; }

        template<typename TT>
        void return_value(TT&& val)
        {
            m_promise.set_value(::std::forward<TT>(val));
            if (m_caller)
            {
                task_scheduler_concept auto& scheduler = get_task_scheduler();
                scheduler.enqueue(m_caller);
            }
        }

    private:
        ::std::promise<T> m_promise;
        ::std::coroutine_handle<> m_caller{};
    };

public:
    _type(promise_type& p)
        : get_result_aw<T, _type>(p),
          m_coro_handle{ ::std::coroutine_handle<promise_type>::from_promise(p) }
    {
    }

    ~_type() noexcept
    {
        if (m_coro_handle) m_coro_handle.destroy();
    }

    void operator()() 
    {
        if (m_coro_handle) [[likely]]
        {
            m_coro_handle();
        }
    }

    bool done() const noexcept 
    {
        if (m_coro_handle) 
            return m_coro_handle.done();
        return true;
    }

    auto result()
    {
        if constexpr (::std::same_as<value_type, void>)
            get_result_aw<T, _type>::future().get();
        else return get_result_aw<T, _type>::future().get();
    }

    auto move_out_coro_handle() noexcept
    {
        return ::std::exchange(m_coro_handle, nullptr);
    }

private:
    ::std::coroutine_handle<promise_type> m_coro_handle;
};

template<typename T>
using task = typename _task<T>::_type;

KOIOS_NAMESPACE_END

#endif
