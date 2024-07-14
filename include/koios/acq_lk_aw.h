// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

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
