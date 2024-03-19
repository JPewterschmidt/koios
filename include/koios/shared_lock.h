/* Koios, A c++ async runtime library.
 * Copyright (C) 2023  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KOIOS_SHARED_LOCK_H
#define KOIOS_SHARED_LOCK_H

#include <cassert>
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

        assert(!this->is_hold());
        this->m_hold = ::std::exchange(lk.m_hold, false);
        assert(this->is_hold());

        co_return;
    }
};

} // namespace koios

#endif
