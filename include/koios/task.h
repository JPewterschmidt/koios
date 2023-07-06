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
class _task<T, DriverPolicy>::_type : public get_result_aw<T, _task<T, DriverPolicy>::_type>
{
public:
    using value_type = T;

    class promise_type 
        : public promise_base, 
          public return_value_or_void<T, promise_type>
    {
    public:
        _task<T, DriverPolicy>::_type get_return_object() noexcept
        {
            return { *this };
        }
    };

public:
    _type(promise_type& p)
        : get_result_aw<T, _type>(p),
          m_coro_handle{ ::std::coroutine_handle<promise_type>::from_promise(p) }
    {
    }

    ~_type() noexcept
    {
        if (m_coro_handle) 
        {
            m_coro_handle.destroy();
            m_coro_handle = nullptr; // debug
        }
    }

    _type(_type&& other) noexcept
        : get_result_aw<T, _task<T, DriverPolicy>::_type>(::std::move(other)),
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
            get_result_aw<T, _type>::future().get();
        else return get_result_aw<T, _type>::future().get();
    }

    auto& future()
    {
        return get_result_aw<T, _type>::future();
    }

    auto move_out_coro_handle() noexcept
    {
        return ::std::exchange(m_coro_handle, nullptr);
    }

    // ================== user friendly

    void run()
    {
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
