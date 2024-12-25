/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
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

#ifndef KOIOS_UNIQUE_LOCK_H
#define KOIOS_UNIQUE_LOCK_H

#include <utility>
#include "koios/task.h"
#include "koios/lock_base.h"

namespace koios
{

/*! \brief  The RAII object which holds the ownership of corresponding mutex. 
 *
 *  This type of object should be generated by a `koios::mutex` object.
 */
template<typename Mutex>
class unique_lock : public lock_base<Mutex>
{
public:
    using lock_base<Mutex>::lock_base;

    /*! \brief Regain the ownership of the corresponding mutex (asynchronous) */
    task<> 
    lock()
    {
        if (!this->m_mutex) [[unlikely]]
            throw koios::exception{ "there's no corresponding mutex instance!" };

        toolpex_assert(!this->owns_lock());
        auto lk = co_await this->m_mutex->acquire();
        this->m_owns = ::std::exchange(lk.m_owns, false);

        co_return;
    }

    task<bool>
    try_lock()
    {
        if (!this->m_mutex) [[unlikely]]
            throw koios::exception{ "there's no corresponding mutex instance!" };

        toolpex_assert(!this->owns_lock());
        auto lk_opt = co_await this->m_mutex->try_acquire();
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
