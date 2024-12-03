// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_COROUTINE_SHARED_MUTEX_H
#define KOIOS_COROUTINE_SHARED_MUTEX_H

#include <mutex>

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

    try_acq_shr_lk_aw<shared_mutex> try_acquire_shared() noexcept { return { *this }; }
    try_acq_lk_aw<shared_mutex> try_acquire() noexcept { return { *this }; }

private:
    template<typename T>
    friend class lock_base;

    template<typename T> friend class acq_lk_aw;
    template<typename T> friend class acq_shr_lk_aw;
    template<typename T> friend class try_acq_lk_aw;
    template<typename T> friend class try_acq_shr_lk_aw;

    void try_wake_up_next() noexcept;

    bool hold_this_shr_immediately() noexcept;
    void add_shr_waiting(task_on_the_fly t);

    bool hold_this_immediately() noexcept;
    void add_waiting(task_on_the_fly t);

    void release();

    enum holding_state { NO = 0, SHR, UNI, SHR_DURING_UNI };

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
    ::std::mutex m_lock;
    holding_state m_state{ holding_state::NO };
    toolpex::ref_count m_shr_cnt{0};
    void* m_current_writer{};
};

} // namespace koios 

#endif
