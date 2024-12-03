// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_SHARED_LOCK_H
#define KOIOS_SHARED_LOCK_H

#include "toolpex/assert.h"

#include "koios/exceptions.h"
#include "koios/lock_base.h"

namespace koios
{

template<typename Mutex>
class shared_lock : public lock_base<Mutex>
{
public:
    task<> lock()
    {
        if (!this->m_mutex) [[unlikely]]
            throw koios::exception{ "there's no corresponding shared mutex instance!" };

        toolpex_assert(!this->owns_lock());
        auto lk = co_await this->m_mutex->acquire_shared();
        this->m_owns = ::std::exchange(lk.m_owns, false);

        co_return;
    }

    task<bool>
    try_lock()
    {
        if (!this->m_mutex) [[unlikely]]
            throw koios::exception{ "there's no corresponding shared mutex instance!" };

        toolpex_assert(!this->owns_lock());
        auto lk_opt = co_await this->m_mutex->try_acquire_shared();
        if (lk_opt) 
        {
            this->m_owns = ::std::exchange(lk_opt.value().m_owns, false);
            co_return true;
        }
        co_return false;
    }
};

} // namespace koios

#endif
