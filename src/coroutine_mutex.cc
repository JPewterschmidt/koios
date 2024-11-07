// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/coroutine_mutex.h"
#include "koios/runtime.h"
#include "koios/exceptions.h"

KOIOS_NAMESPACE_BEG

void mutex::
try_wake_up_next() noexcept
{
    if (!hold_this_immediately())
        return;

    waiting_handle handle{};
    if (m_waitings.try_dequeue(handle))
    {
        wake_up(handle);
    }
    else
    {
        m_flag.clear(::std::memory_order_release);
    }
}

bool mutex::
be_held() noexcept
{
    if (hold_this_immediately())
    {
        release();
        return false;
    }
    return true;
}

KOIOS_NAMESPACE_END
