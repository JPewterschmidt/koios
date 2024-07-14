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

        auto lk = co_await this->m_mutex->acquire_shared();

        toolpex_assert(!this->is_hold());
        this->m_hold = ::std::exchange(lk.m_hold, false);
        toolpex_assert(this->is_hold());

        co_return;
    }
};

} // namespace koios

#endif
