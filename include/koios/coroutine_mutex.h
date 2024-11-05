// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_COROUTINE_MUTEX_H
#define KOIOS_COROUTINE_MUTEX_H

#include <mutex>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/per_consumer_attr.h"
#include "koios/task.h"
#include "koios/unique_lock.h"
#include "koios/acq_lk_aw.h"
#include "koios/waiting_handle.h"
#include "toolpex/move_only.h"

#undef BLOCK_SIZE
#include "concurrentqueue/concurrentqueue.h"

KOIOS_NAMESPACE_BEG

/*! \brief The coroutine mutex object 
 *
 *  Based on `::std::mutex`.
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

    try_acq_lk_aw<mutex> try_acquire() noexcept { return { *this }; }

private:
    template<typename T>
    friend class lock_base;

    template<typename T> friend class acq_lk_aw;
    template<typename T> friend class try_acq_lk_aw;

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

private:
    bool being_held_impl() const noexcept { return m_holded; }

private:
    moodycamel::ConcurrentQueue<waiting_handle> m_waitings;

    ::std::mutex m_lock;  
    bool m_holded{false};
};

KOIOS_NAMESPACE_END

#endif
