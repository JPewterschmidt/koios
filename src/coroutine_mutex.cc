// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

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
