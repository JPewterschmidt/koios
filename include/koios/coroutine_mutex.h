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

#ifndef KOIOS_COROUTINE_MUTEX_H
#define KOIOS_COROUTINE_MUTEX_H

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/per_consumer_attr.h"
#include "koios/task.h"
#include "koios/unique_lock.h"
#include "koios/acq_lk_aw.h"
#include "koios/waiting_handle.h"
#include "toolpex/move_only.h"
#include "toolpex/spin_lock.h"

#undef BLOCK_SIZE
#include "concurrentqueue/concurrentqueue.h"

KOIOS_NAMESPACE_BEG

/*! \brief The coroutine mutex object 
 *
 *  Based on a spin lock.
 */
class mutex : public toolpex::move_only
{
public:
    /*! \brief  Acquire the ownership of the mutex (asynchronous)
     *
     *  Due to the underlying mutex may throw an exception when lock it, 
     *  We didn't mark this function as noexcept.
     */
    acq_lk_aw<mutex> acquire() noexcept { return { *this }; }

private:
    template<typename T>
    friend class lock_base;

    template<typename T>
    friend class acq_lk_aw;

    /*! \brief Wake up the next waiting task handler
     *
     *  This function won't check the ownership, 
     *  and will be called by `release()` member function
     *  
     *  \see `release()`
     */
    void try_wake_up_next() noexcept;

    /*! \brief  Try to acquire the ownership.
     *  \retval true The ownership was got.
     *  \retval false The ownership was not got.
     */
    bool hold_this_immediately() noexcept;
    void add_waiting(task_on_the_fly t);

    /*! \brief  Move the ownership to the next waiting task, and wake it.
     *
     *  Actually there's no such code deal with ownership, 
     *  ownership of this coroutine mutex implementation 
     *  is just abstract concept. We only guarantee that
     *  there's always one (or no) member task is running of this mutex.
     */
    void release();
    
    void try_wake_up_next_impl() noexcept;

public:
    bool being_held() const noexcept { return m_holded; }

private:
    moodycamel::ConcurrentQueue<waiting_handle> m_waitings;

    toolpex::spin_lock m_lock;  
    bool m_holded{false};
};

KOIOS_NAMESPACE_END

#endif
