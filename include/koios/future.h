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
    task_on_the_fly m_waitting;
    ::std::unique_ptr<toolpex::future_frame<T>> m_ff;

public:
    using sptr = ::std::shared_ptr<shared_state>;

    void set_value(toolpex::future_frame<T> ff)
    {
        ::std::lock_guard lk{ m_lock };
        m_ff = ::std::make_unique<toolpex::future_frame<T>>(::std::move(ff));
        if (m_waitting)
            wake_up(::std::move(m_waitting));
    }

    void set_waitting(task_on_the_fly t)
    {
        ::std::lock_guard lk{ m_lock };
        toolpex_assert(!m_waitting);
        m_waitting = ::std::move(t);
        if (m_ff)
            wake_up(::std::move(m_waitting));
    }

    auto get_future_frame_ptr()
    {
        ::std::lock_guard lk{ m_lock };
        return ::std::move(m_ff);
    }
};

} // namespace future_detials

template<typename T>
class future
{
public:
    constexpr future() noexcept = default;

    future(future_detials::shared_state<T>::sptr shared_state) noexcept
        : m_ss{ ::std::move(shared_state) }
    {
        toolpex_assert(!!m_ss);
    }

    future_aw<future> get_async() { return { *this }; }

    decltype(auto) get_nonblk() 
    {
        if (!m_ff) [[unlikely]]
        {
            throw ::std::out_of_range{ "future::get_nonblk(): not ready!" };
        }

        auto& ff = *m_ff;
        if (!ff.safely_done())
        {
            ::std::rethrow_exception(ff.exception());
        }
        return ff.value();
    }

    bool ready()
    {
        if (!m_ff) 
            m_ff = m_ss->get_future_frame_ptr();
        return m_ff;
    }

    void set_waitting(task_on_the_fly f)
    {
        m_ss->set_waitting(::std::move(f));
    }

    bool vaild() const noexcept
    {
        return !!m_ss;
    }

private:
    future_detials::shared_state<T>::sptr m_ss;
    ::std::unique_ptr<toolpex::future_frame<T>> m_ff;
};

template<typename T>
class promise
{
public:
    promise() noexcept
        : m_cp(get_wakeup_func())
    {
    }

    future<T> get_future()
    {
        return { m_shared_state };
    }

    template<typename... Args>
    void set_value(Args&&... args)
    {
        m_cp->set_value(::std::forward<Args>(args)...);
    }

private:
    auto get_wakeup_func() noexcept
    {
        return [this](toolpex::future_frame<T> ff){
            m_shared_state->set_value(::std::move(ff));
        };
    }

private:
    future_detials::shared_state<T>::sptr m_shared_state{ ::std::make_shared<future_detials::shared_state<T>>() };
    toolpex::callback_promise<T> m_cp;
};


KOIOS_NAMESPACE_END

#endif
