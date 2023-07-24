#ifndef KOIOS_TASK_SCHEDULER_H
#define KOIOS_TASK_SCHEDULER_H

#include <coroutine>
#include <concepts>

#include "koios/macros.h"
#include "koios/task_concepts.h"
#include "koios/thread_pool.h"
#include "koios/task_on_the_fly.h"
#include "koios/std_queue_wrapper.h"
#include "koios/moodycamel_queue_wrapper.h"

KOIOS_NAMESPACE_BEG

/*! \brief the async task scheduler class. */
class task_scheduler : public thread_pool
{
public:
    using queue_type = moodycamel_queue_wrapper;

public:
    /*! \param thr_cnt the number of thread you want.
     *  \param manually_stop_type a tag which indecate 
     *                            your willing of how the destructor behave. Just like `thread_pool`.
     */
    explicit task_scheduler(size_t thr_cnt, manually_stop_type)
        : thread_pool{ thr_cnt, queue_type{}, manually_stop }
    {
    }

    explicit task_scheduler(size_t thr_cnt)
        : thread_pool{ thr_cnt, queue_type{} }
    {
    }

    void enqueue(task_concept auto t) noexcept
    {
        enqueue(t.move_out_coro_handle());
    }

    void enqueue(task_on_the_fly h) noexcept
    {
        if (!h) [[unlikely]] return;
        thread_pool::enqueue_no_future_without_checking([h = ::std::move(h)] mutable { h(); });
    }

    void stop() noexcept { thread_pool::stop(); }
    void quick_stop() noexcept { thread_pool::quick_stop(); }
};

KOIOS_NAMESPACE_END

#endif
