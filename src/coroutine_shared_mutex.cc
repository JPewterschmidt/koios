#include "koios/coroutine_shared_mutex.h"
#include <mutex>

namespace koios
{

void shared_mutex::try_wake_up_next() noexcept
{
    ::std::lock_guard lk{ m_lock };
    try_wake_up_next_uni_impl();
    if (being_held()) return;
    try_wake_up_shr_impl();
}

bool shared_mutex::hold_this_shr_immediately() noexcept
{
	::std::lock_guard lk{ m_lock };
    if (!being_held())
    {
        m_shr_cnt.fetch_add();
        m_state = SHR;
        return true;
    }
    return false;
}

void shared_mutex::add_shr_waiting(task_on_the_fly t)
{
	m_shr_waitings.enqueue({ .task = ::std::move(t) });
}

bool shared_mutex::hold_this_immediately() noexcept
{
    ::std::lock_guard lk{ m_lock };
    return (!being_held() && !being_held_sharedly()) ? (m_state = UNI, true) : false;
}

void shared_mutex::add_waiting(task_on_the_fly t)
{
	m_uni_waitings.enqueue({ .task = ::std::move(t) });
}

void shared_mutex::release()
{
    ::std::lock_guard lk{ m_lock };
	switch (m_state)
    {
    case SHR: if (m_shr_cnt.fetch_sub() == 1)
              {
                  m_state = NO;
                  try_wake_up_next_uni_impl();
              }
              break;

    case UNI: m_state = NO;
              try_wake_up_shr_impl();
              if (!being_held_sharedly()) 
                  try_wake_up_next_uni_impl();
              break;
    case NO:  assert(false);
    }
    if (!being_held() && !being_held_sharedly()) m_state = NO;
}

bool shared_mutex::being_held() const noexcept
{
    return m_state == UNI;
}

bool shared_mutex::being_held_sharedly() const noexcept
{
    return m_state == SHR;
}

void shared_mutex::try_wake_up_next_uni_impl() noexcept
{
    if (being_held() || being_held_sharedly()) return;
    waiting_handle handle{};
    if (m_uni_waitings.try_dequeue(handle))
    {
        m_state = UNI;
        wake_up(handle);
    }
}

void shared_mutex::try_wake_up_shr_impl() noexcept
{
    if (being_held()) return;
    waiting_handle handle{};
    if (m_shr_waitings.try_dequeue(handle))
    {
        m_state = SHR;
        wake_up(handle);
    }
}

} // namespace koios
