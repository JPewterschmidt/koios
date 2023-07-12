#ifndef KOIOS_TASK_H
#define KOIOS_TASK_H

#include <utility>
#include <memory>

#include "koios/macros.h"
#include "koios/promise_base.h"
#include "koios/promise_wrapper.h"
#include "koios/return_value_or_void.h"
#include "koios/task_scheduler_wrapper.h"
#include "koios/get_result_aw.h"
#include "koios/driver_policy.h"

KOIOS_NAMESPACE_BEG

template<typename, driver_policy_concept, typename>
struct _task
{
    struct [[nodiscard]] _type;
};

struct discardable{};
struct non_discardable{};

template<
    typename T, 
    driver_policy_concept DriverPolicy,
    typename Discardable>
class _task<T, DriverPolicy, Discardable>::_type 
    : public get_result_aw<T, _task<T, DriverPolicy, Discardable>::_type, DriverPolicy>
{
public:
    using value_type = T;

    class promise_type 
        : public promise_base<>, 
          public return_value_or_void<T, promise_type, DriverPolicy>
    {
    public:
        _task<T, DriverPolicy, Discardable>::_type get_return_object() noexcept
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
        if (m_need_destroy_in_dtor && m_coro_handle) [[unlikely]] m_coro_handle.destroy();
    }

    _type(_type&& other) noexcept
        : get_result_aw<T, _task<T, DriverPolicy, Discardable>::_type, DriverPolicy>(::std::move(other)),
          m_coro_handle{ other.m_coro_handle }, 
          m_need_destroy_in_dtor{ ::std::exchange(other.m_need_destroy_in_dtor, false) }
    {
    }

    void operator()() 
    {
        run();
    }

    bool done() const noexcept 
    {
        if (m_coro_handle) 
            return m_coro_handle.done();
        return true;
    }

    auto& future()
    {
        if (has_scheduled())
            throw ::std::logic_error{ "You should call `future()` before `run()`" };

        // prevent the `::std::promise` object being destroied by `coro.destroy()`
        m_std_promise_p = m_coro_handle.promise().get_std_promise_pointer();

        return get_result_aw<T, _type, DriverPolicy>::future();
    }

    auto move_out_coro_handle() noexcept
    {
        return ::std::exchange(m_coro_handle, nullptr);
    }

    // ================== user friendly

    void run()
    {
        if constexpr (!is_discardable())
        {
            if (!has_got_future())
            {
                throw ::std::logic_error{ 
                    "This task is non-discardable task, "
                    "You have too get this future object before run it!" 
                };
            }
        }
        if (!::std::exchange(m_need_destroy_in_dtor, false)) return;
        DriverPolicy{}.scheduler().enqueue(move_out_coro_handle());
    }

    [[nodiscard]] static constexpr bool is_discardable() 
    {
        return ::std::same_as<Discardable, discardable>;
    }

private:
    [[nodiscard]] bool has_got_future() const noexcept { return bool(m_std_promise_p); }
    [[nodiscard]] bool has_scheduled() const noexcept { return !m_need_destroy_in_dtor; }

private:
    ::std::coroutine_handle<promise_type> m_coro_handle;
    ::std::shared_ptr<::std::promise<value_type>> m_std_promise_p{};
    bool m_need_destroy_in_dtor{ true };
};

template<typename T>
using async_task = typename _task<T, run_this_async, discardable>::_type;

template<typename T>
using sync_task = typename _task<T, run_this_sync, discardable>::_type;

template<typename T>
using task = async_task<T>;

template<typename T>
using nodiscard_task = typename _task<T, run_this_async, non_discardable>::_type;

KOIOS_NAMESPACE_END

#endif
