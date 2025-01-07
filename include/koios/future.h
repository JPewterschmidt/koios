// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_FUTURE_H
#define KOIOS_FUTURE_H

#include <atomic>
#include <exception>
#include <memory>
#include <type_traits>
#include <optional>
#include <condition_variable> 
#include <mutex>

#include "toolpex/assert.h"
#include "toolpex/callback_promise.h"

#include "koios/macros.h"
#include "koios/exceptions.h"
#include "koios/future_aw.h"
#include "koios/waiting_handle.h"

KOIOS_NAMESPACE_BEG

namespace future_detials
{

template<typename T>
class shared_state
{
private:
    mutable ::std::mutex m_lock;
    ::std::condition_variable m_cond;
    task_on_the_fly m_waiting;
    ::std::unique_ptr<toolpex::future_frame<T>> m_ff;

public:
    using sptr = ::std::shared_ptr<shared_state>;

    void set_value(toolpex::future_frame<T> ff)
    {
        ::std::lock_guard lk{ m_lock };
        m_ff = ::std::make_unique<toolpex::future_frame<T>>(::std::move(ff));
        if (m_waiting)
            wake_up(::std::move(m_waiting));
        m_cond.notify_one();
    }

    void set_waiting(task_on_the_fly t)
    {
        ::std::lock_guard lk{ m_lock };
        toolpex_assert(!m_waiting);
        m_waiting = ::std::move(t);
        if (m_ff)
            wake_up(::std::move(m_waiting));
    }

    auto get_future_frame_ptr()
    {
        ::std::lock_guard lk{ m_lock };
        return ::std::move(m_ff);
    }

    void wait_util_has_value()
    {
        ::std::unique_lock lk{ m_lock };
        if (m_ff) return;
        m_cond.wait(lk);
    }
};

} // namespace future_detials

template<typename T>
class future
{
public:
    using value_type = T;

public:
    constexpr future() noexcept = default;

    future(future_detials::shared_state<T>::sptr shared_state) noexcept
        : m_ss{ ::std::move(shared_state) }
    {
        toolpex_assert(!!m_ss);
    }

    future_aw<future> get_async() 
    { 
        toolpex_assert(valid());
        return { *this }; 
    }

    future_aw<future> operator co_await ()
    {
        return get_async();
    }

    value_type get_nonblk() 
    {
        toolpex_assert(valid());
        if (!m_ff)
        {
            m_ff = m_ss->get_future_frame_ptr();
        }
        if (!m_ff) [[unlikely]]
        {
            throw ::std::out_of_range{ "future::get_nonblk(): not ready!" };
        }
        if (m_has_got) 
        {
            throw ::std::logic_error{ "future::get_nonblk(): has been got." };
        }

        m_has_got = true;
        auto& ff = *m_ff;
        if (!ff.safely_done())
        {
            ::std::rethrow_exception(ff.get_exception());
        }
        if constexpr (!::std::same_as<void, value_type>)
        {
            if constexpr (::std::is_reference_v<value_type>)
                return ff.value();
            else return ::std::move(ff.value());
        }
    }

    value_type get()
    {
        toolpex_assert(valid());
        if (m_has_got) 
        {
            throw ::std::logic_error{ "future::get(): has been got." };
        }
        m_ss->wait_util_has_value();
        if (!m_ff) 
            m_ff = m_ss->get_future_frame_ptr();
        return get_nonblk();
    }

    bool ready()
    {
        toolpex_assert(valid());
        if (!m_ff) 
            m_ff = m_ss->get_future_frame_ptr();
        return !!m_ff;
    }

    void set_waiting(task_on_the_fly f)
    {
        toolpex_assert(valid());
        m_ss->set_waiting(::std::move(f));
    }

    bool valid() const noexcept
    {
        return !!m_ss;
    }

private:
    future_detials::shared_state<T>::sptr m_ss;
    ::std::unique_ptr<toolpex::future_frame<T>> m_ff;
    bool m_has_got{};
};

template<typename T>
class promise
{
public:
    using value_type = T;

public:
    promise() noexcept
        : m_shared_state{ ::std::make_shared<future_detials::shared_state<T>>() },
          m_cp(get_wakeup_func())
    {
    }

    future<T> get_future()
    {
        return { m_shared_state };
    }

    template<typename... Args>
    void set_value(Args&&... args)
    {
        m_cp.set_value(::std::forward<Args>(args)...);
    }

    void set_exception(::std::exception_ptr ex)
    {
        m_cp.set_exception(::std::move(ex));
    }

private:
    auto get_wakeup_func() noexcept
    {
        toolpex_assert(!!m_shared_state);
        return [shared_state = m_shared_state](toolpex::future_frame<T> ff){
            shared_state->set_value(::std::move(ff));
        };
    }

private:
    future_detials::shared_state<T>::sptr m_shared_state{};
    toolpex::callback_promise<T> m_cp;
};


KOIOS_NAMESPACE_END

#endif
