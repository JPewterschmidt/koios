#ifndef KOIOS_COROUTINE_MUTEX_H
#define KOIOS_COROUTINE_MUTEX_H

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/per_consumer_attr.h"
#include "koios/task.h"

#undef BLOCK_SIZE
#include "concurrentqueue/concurrentqueue.h"

#include <mutex>

KOIOS_NAMESPACE_BEG

class mutex;

class unique_lock
{
public:
    unique_lock(mutex& m) noexcept
        : m_mutex{ m }
    {
    }

    task<void> lock();
    void unlock() noexcept;
    ~unique_lock() noexcept { unlock(); }

private:
    mutex& m_mutex;
    bool m_hold{ true };
};

class acq_lk_aw
{
public:
    acq_lk_aw(mutex& mutex) noexcept
        : m_mutex{ mutex }
    {
    }
    
    bool await_ready() const;
    void await_suspend(task_on_the_fly h);
    unique_lock await_resume() noexcept;

private:
    mutex& m_mutex;
};

struct waiting_handle
{
    per_consumer_attr attr;
    task_on_the_fly task;
};

class mutex
{
public:
    acq_lk_aw acquire() { return { *this }; }

private:
    friend class unique_lock;
    friend class acq_lk_aw;

    // reentrant
    void try_wake_up_next() noexcept;
    bool hold_this_immediately();
    void add_waiting(task_on_the_fly t);
    void release();
    
    void try_wake_up_next_impl() noexcept;

private:
    moodycamel::ConcurrentQueue<waiting_handle> m_waitings;

    ::std::mutex m_lock;  
    bool m_holded{false};
};

KOIOS_NAMESPACE_END

#endif
