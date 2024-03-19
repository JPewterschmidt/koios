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

#ifndef KOIOS_COROUTINE_SHARED_MUTEX_H
#define KOIOS_COROUTINE_SHARED_MUTEX_H

#include "koios/unique_lock.h"
#include "koios/shared_lock.h"
#include "koios/acq_lk_aw.h"
#include "koios/waiting_handle.h"

#undef BLOCK_SIZE
#include "concurrentqueue/concurrentqueue.h"

#include "toolpex/ref_count.h"

namespace koios
{

class shared_mutex : public toolpex::move_only
{
public:
    acq_shr_lk_aw<shared_mutex> acquire_shared() noexcept { return { *this }; }
    acq_lk_aw<shared_mutex> acquire() noexcept { return { *this }; }

private:
    template<typename T>
    friend class lock_base;

    template<typename T>
    friend class acq_lk_aw;
    
    template<typename T>
    friend class acq_shr_lk_aw;

    void try_wake_up_next() noexcept;

    bool hold_this_shr_immediately() noexcept;
    void add_shr_waiting(task_on_the_fly t);

    bool hold_this_immediately() noexcept;
    void add_waiting(task_on_the_fly t);

    void release();

    enum holding_state { NO, SHR, UNI, };

private:
    bool being_held() const noexcept;
    bool being_held_sharedly() const noexcept;
    void try_wake_up_next_uni_impl() noexcept;
    void try_wake_up_shr_impl() noexcept;
    
private:
    bool health_check() const noexcept;

private:
    moodycamel::ConcurrentQueue<waiting_handle> m_shr_waitings;
    moodycamel::ConcurrentQueue<waiting_handle> m_uni_waitings;
    toolpex::spin_lock m_lock;
    holding_state m_state;
    toolpex::ref_count m_shr_cnt{0};
};

} // namespace koios 

#endif
