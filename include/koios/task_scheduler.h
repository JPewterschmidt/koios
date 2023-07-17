#ifndef KOIOS_TASK_SCHEDULER_H
#define KOIOS_TASK_SCHEDULER_H

#include <coroutine>
#include <concepts>

#include "koios/macros.h"
#include "koios/task_concepts.h"
#include "koios/thread_pool.h"
#include "koios/moodycamel_queue_wrapper.h"

KOIOS_NAMESPACE_BEG

class task_scheduler : public thread_pool
{
public:
    explicit task_scheduler(size_t thr_cnt, manually_stop_type)
        : thread_pool{ thr_cnt, moodycamel_queue_wrapper{}, manually_stop }
    {
    }

    explicit task_scheduler(size_t thr_cnt)
        : thread_pool{ thr_cnt, moodycamel_queue_wrapper{} }
    {
    }

    void enqueue(task_concept auto t)
    {
        enqueue(t.move_out_coro_handle());
    }

    void enqueue(::std::coroutine_handle<> h)
    {
        if (h) [[likely]]
        {
            thread_pool::enqueue_no_future([h]() noexcept { 
                try { h(); } 
                catch (...) {}
            });
        }
    }

    void stop() noexcept { thread_pool::stop(); }
    void quick_stop() noexcept { thread_pool::quick_stop(); }
};

KOIOS_NAMESPACE_END

#endif
