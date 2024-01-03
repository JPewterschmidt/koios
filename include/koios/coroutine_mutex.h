#ifndef KOIOS_COROUTINE_MUTEX_H
#define KOIOS_COROUTINE_MUTEX_H

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/per_consumer_attr.h"
#include "koios/task.h"
#include "toolpex/move_only.h"

#undef BLOCK_SIZE
#include "concurrentqueue/concurrentqueue.h"

#include <mutex>

KOIOS_NAMESPACE_BEG

class mutex;

/*! \brief  The RAII object which holds the ownership of corresponding mutex. 
 *
 *  This type of object should be generated by a `koios::mutex` object.
 */
class unique_lock : public toolpex::move_only
{
private:
    friend class mutex;
    friend class acq_lk_aw;

    unique_lock(mutex& m) noexcept
        : m_mutex{ &m }
    {
    }

public:
    unique_lock(unique_lock&& other) noexcept;
    unique_lock& operator=(unique_lock&& other) noexcept;

    /*! \brief Regain the ownership of the corresponding mutex (asynchronous) */
    task<void> lock();

    /*! \brief Give up the ownership of the corresponding mutex.
     *  
     *  After give up the ownership, you can `lock()`
     *  corresponding the corresponding mutex.
     */
    void unlock() noexcept;

    /*! \brief Automatically release the ownership. */
    ~unique_lock() noexcept { unlock(); }

private:
    mutex* m_mutex;
    bool m_hold{ true };
};

class acq_lk_aw : public toolpex::move_only
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

/*! \brief The coroutine mutex object */
class mutex : public toolpex::move_only
{
public:
    /*! \brief  Acquire the ownership of the mutex (asynchronous)*/
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
