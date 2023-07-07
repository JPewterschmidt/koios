#ifndef KOIOS_TASK_H
#define KOIOS_TASK_H

#include <utility>

#include "koios/macros.h"
#include "koios/promise_base.h"
#include "koios/promise_wrapper.h"
#include "koios/return_value_or_void.h"
#include "koios/task_scheduler_wrapper.h"
#include "koios/get_result_aw.h"
#include "koios/driver_policy.h"

KOIOS_NAMESPACE_BEG

template<typename T, driver_policy_concept DriverPolicy>
struct _task
{
    struct [[nodiscard]] _type;
};

template<
    typename T, 
    driver_policy_concept DriverPolicy>
class _task<T, DriverPolicy>::_type : public get_result_aw<T, _task<T, DriverPolicy>::_type, DriverPolicy>
{
public:
    using value_type = T;

    class promise_type 
        : public promise_base, 
          public return_value_or_void<T, promise_type, DriverPolicy>
    {
    public:
        _task<T, DriverPolicy>::_type get_return_object() noexcept
        {
            return { *this };
        }
    };

private:
    _type(promise_type& p)
        : get_result_aw<T, _type, DriverPolicy>(p),
          m_coro_handle{ ::std::coroutine_handle<promise_type>::from_promise(p) }
    {
    }

public:
    ~_type() noexcept
    {
        if (m_coro_handle) 
        {
            m_coro_handle.destroy();
            m_coro_handle = nullptr;
        }
    }

    _type(_type&& other) noexcept
        : get_result_aw<T, _task<T, DriverPolicy>::_type, DriverPolicy>(::std::move(other)),
          m_coro_handle{ ::std::exchange(other.m_coro_handle, nullptr) }
    {
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
            get_result_aw<T, _type, DriverPolicy>::future().get();
        else return get_result_aw<T, _type, DriverPolicy>::future().get();
    }

    auto& future()
    {
        return get_result_aw<T, _type, DriverPolicy>::future();
    }

    auto move_out_coro_handle() noexcept
    {
        return ::std::exchange(m_coro_handle, nullptr);
    }

    // ================== user friendly

    void run()
    {
        if (done()) return;
        DriverPolicy{}.scheduler().enqueue(move_out_coro_handle());
    }

private:
    ::std::coroutine_handle<promise_type> m_coro_handle;
};

template<typename T>
using async_task = typename _task<T, run_this_async>::_type;

template<typename T>
using sync_task = typename _task<T, run_this_sync>::_type;

template<typename T>
using task = async_task<T>;

KOIOS_NAMESPACE_END

#endif
