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
#include <mutex>

KOIOS_NAMESPACE_BEG

bool mutex::
hold_this_immediately() noexcept
{
    ::std::lock_guard lk{ m_lock };
    return m_holded ? false : (m_holded = true);
}

void mutex::
add_waiting(task_on_the_fly h)
{
    m_waitings.enqueue({ .task = ::std::move(h) });
}

void mutex::
try_wake_up_next_impl() noexcept
{
    if (being_held_impl()) return;

    waiting_handle handle{};
    if (m_waitings.try_dequeue(handle))
    {
        m_holded = true;
        wake_up(handle);
    }
    else m_holded = false;
}

void mutex::
try_wake_up_next() noexcept
{
    ::std::lock_guard lk{ m_lock };
    this->try_wake_up_next_impl();
}

void
mutex::
release()
{
    ::std::lock_guard lk{ m_lock };
    m_holded = false;
    this->try_wake_up_next_impl();
}

KOIOS_NAMESPACE_END
