// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/coroutine_shared_mutex.h"
#include <mutex>

namespace koios
{

void shared_mutex::try_wake_up_next() noexcept
{
    ::std::lock_guard lk{ m_lock };
    this->try_wake_up_next_uni_impl();
    if (this->being_held()) return;
    this->try_wake_up_shr_impl();
}

bool shared_mutex::hold_this_shr_immediately() noexcept
{
	::std::lock_guard lk{ m_lock };
    if (!this->being_held())
    {
        m_shr_cnt.fetch_add();
        m_state = SHR;
        return true;
    }
    return false;
}

void shared_mutex::add_shr_waiting(task_on_the_fly t)
{
    ::std::lock_guard lk{ m_lock };
    if (t.address() == m_current_writer)
    {
        m_state = SHR_DURING_UNI;
        get_task_scheduler().enqueue(::std::move(t));
    }
    else
    {
        m_shr_waitings.enqueue({ .task = ::std::move(t) });
    }
}

bool shared_mutex::hold_this_immediately() noexcept
{
    return false;
}

void shared_mutex::add_waiting(task_on_the_fly t)
{
    ::std::lock_guard lk{ m_lock };
    const bool have_ownership = (!this->being_held() && !this->being_held_sharedly()) ? (m_state = UNI, true) : false;
    if (have_ownership)
    {
        m_current_writer = t.address();
        get_task_scheduler().enqueue(::std::move(t));
    }
    else
    {
        m_uni_waitings.enqueue({ .task = ::std::move(t) });
    }
}

void shared_mutex::release()
{
    ::std::lock_guard lk{ m_lock };
	switch (m_state)
    {
    case SHR: if (m_shr_cnt.fetch_sub() == 1)
              {
                  m_state = NO;
                  this->try_wake_up_next_uni_impl();
              }
              break;

    case UNI: m_state = NO;
              m_current_writer = nullptr;
              this->try_wake_up_shr_impl();
              if (!this->being_held_sharedly()) 
                  this->try_wake_up_next_uni_impl();
              break;

    case SHR_DURING_UNI:
              m_state = UNI;
              return;

    case NO:  assert(false);
    }
    if (!this->being_held() && !this->being_held_sharedly()) m_state = NO;
}

bool shared_mutex::being_held() const noexcept
{
    assert(this->health_check());
    return m_state == UNI;
}

bool shared_mutex::health_check() const noexcept
{
    return !(m_state == UNI && m_shr_cnt.load() != 0);
}

bool shared_mutex::being_held_sharedly() const noexcept
{
    assert(this->health_check());
    return m_state == SHR || m_state == SHR_DURING_UNI;
}

void shared_mutex::try_wake_up_next_uni_impl() noexcept
{
    if (this->being_held() || this->being_held_sharedly()) return;
    waiting_handle handle{};
    if (m_uni_waitings.try_dequeue(handle))
    {
        m_state = UNI;
        m_current_writer = handle.task.address();
        wake_up(handle);
    }
}

void shared_mutex::try_wake_up_shr_impl() noexcept
{
    if (this->being_held()) return;
    waiting_handle handle{};
    if (m_shr_waitings.try_dequeue(handle))
    {
        m_state = SHR;
        wake_up(handle);
    }
}

} // namespace koios
