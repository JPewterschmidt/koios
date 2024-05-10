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

#ifndef KOIOS_ACQ_LK_AW_H
#define KOIOS_ACQ_LK_AW_H

#include "koios/unique_lock.h"
#include "koios/shared_lock.h"
#include "toolpex/move_only.h"

namespace koios
{

template<typename Mutex>
class acq_lk_aw : public toolpex::move_only
{
public:
    acq_lk_aw(Mutex& mutex) noexcept
        : m_mutex{ mutex }
    {
    }
    
    void await_suspend(task_on_the_fly h)
    {
        m_mutex.add_waiting(::std::move(h));
    }

    /*! \brief  Try gain then check the ownership of the lock. */
    bool await_ready() const noexcept
    {
        return m_mutex.hold_this_immediately();
    }

    unique_lock<Mutex> await_resume() noexcept
    {
        return { m_mutex };
    }

private:
    Mutex& m_mutex;
};

template<typename Mutex>
class acq_shr_lk_aw : public toolpex::move_only
{
public:
    acq_shr_lk_aw(Mutex& mutex) noexcept
        : m_mutex{ mutex }
    {
    }
    
    void await_suspend(task_on_the_fly h)
    {
        m_mutex.add_shr_waiting(::std::move(h));
        m_mutex.try_wake_up_next();
    }

    /*! \brief  Try gain then check the ownership of the lock. */
    bool await_ready() const noexcept
    {
        return m_mutex.hold_this_shr_immediately();
    }

    shared_lock<Mutex> await_resume() noexcept
    {
        return { m_mutex };
    }

private:
    Mutex& m_mutex;
};

} // namespace koios

#endif
