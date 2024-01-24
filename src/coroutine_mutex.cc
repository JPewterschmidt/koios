/* koios, a c++ async runtime library.
 * copyright (c) 2024  jeremy pewterschmidt
 * 
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 * 
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 * 
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 */

#include "koios/coroutine_mutex.h"
#include "koios/runtime.h"
#include "koios/exceptions.h"

KOIOS_NAMESPACE_BEG

unique_lock::unique_lock(unique_lock&& other) noexcept
    : m_mutex{ ::std::exchange(other.m_mutex, nullptr) }, 
      m_hold{ ::std::exchange(other.m_hold, false) }
{
}

unique_lock& 
unique_lock::operator=(unique_lock&& other) noexcept
{
    unlock();

    m_mutex = ::std::exchange(other.m_mutex, nullptr);
    m_hold = ::std::exchange(other.m_hold, false);

    return *this;
}

void
acq_lk_aw::
await_suspend(task_on_the_fly h)
{
    m_mutex.add_waiting(::std::move(h));
    m_mutex.try_wake_up_next();
}

bool
acq_lk_aw::
await_ready() const
{
    return m_mutex.hold_this_immediately();
}

unique_lock
acq_lk_aw::
await_resume() noexcept
{
    return { m_mutex };
}

task<void> 
unique_lock::
lock()
{
    if (!m_mutex) [[unlikely]]
        throw koios::exception{ "there's no corresponding mutex instance!" };

    auto lk = co_await m_mutex->acquire();

    assert(!m_hold);
    m_hold = ::std::exchange(lk.m_hold, false);
    assert(m_hold);

    co_return;
}

void unique_lock::
unlock() noexcept
{
    if (m_mutex && m_hold)
    {
        m_mutex->release();
        m_hold = false;
    }
}

// ================================================

bool mutex::
hold_this_immediately()
{
    ::std::lock_guard lk{ m_lock };
    return m_holded ? false : (m_holded = true);
}

void mutex::
add_waiting(task_on_the_fly h)
{
    m_waitings.enqueue({ .task = ::std::move(h) });
}

static void wake_up(waiting_handle& h)
{
    auto t = ::std::move(h.task);
    [[assume(bool(t))]];
    get_task_scheduler().enqueue(h.attr, ::std::move(t));
}

void mutex::
try_wake_up_next_impl() noexcept
{
    if (m_holded) return;
    m_holded = true;

    waiting_handle handle{};
    if (m_waitings.try_dequeue(handle))
    {
        wake_up(handle);
    }
    else m_holded = false;
}

void mutex::
try_wake_up_next() noexcept
{
    ::std::lock_guard lk{ m_lock };
    try_wake_up_next_impl();
}

void
mutex::
release()
{
    ::std::lock_guard lk{ m_lock };
    m_holded = false;
    try_wake_up_next_impl();
}

KOIOS_NAMESPACE_END
