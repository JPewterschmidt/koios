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

#ifndef KOIOS_LOCK_BASE_H
#define KOIOS_LOCK_BASE_H

#include "toolpex/move_only.h"

namespace koios
{

template<typename Mutex>
class lock_base : public toolpex::move_only
{
public:
    lock_base(Mutex& m) noexcept
        : m_mutex{ &m }
    {
    }

    lock_base() noexcept = default;

    lock_base(lock_base&& other) noexcept 
        : m_mutex{ ::std::exchange(other.m_mutex, nullptr) }, 
          m_hold{ ::std::exchange(other.m_hold, false) }
    {
    }

    lock_base& operator=(lock_base&& other) noexcept
    {
        unlock();

        m_mutex = ::std::exchange(other.m_mutex, nullptr);
        m_hold = ::std::exchange(other.m_hold, false);

        return *this;
    }

    /*! \brief Give up the ownership of the corresponding mutex.
     *  
     *  After give up the ownership, you can `lock()`
     *  corresponding the corresponding mutex.
     */
    void unlock() noexcept
    {
        if (m_mutex && is_hold())
        {
            m_mutex->release();
            m_hold = false;
        }
    }

    ~lock_base() noexcept { unlock(); }

    bool is_hold() const noexcept { return m_hold; }

protected:
    Mutex* m_mutex{};
    bool m_hold{ true };
};

} // namespace koios

#endif
