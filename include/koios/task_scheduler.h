// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_TASK_SCHEDULER_H
#define KOIOS_TASK_SCHEDULER_H

#include <coroutine>
#include <concepts>

#include "koios/macros.h"
#include "koios/task_concepts.h"
#include "koios/thread_pool.h"
#include "koios/task_on_the_fly.h"
#include "koios/generator_on_the_fly.h"
#include "koios/std_queue_wrapper.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/work_stealing_queue.h"

KOIOS_NAMESPACE_BEG

/*! \brief the async task scheduler class. */
class task_scheduler : public thread_pool
{
public:
    using queue_type = work_stealing_queue<moodycamel_queue_wrapper>;

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
        this->enqueue(t.move_out_coro_handle());
    }

    void enqueue(task_on_the_fly h) noexcept
    {
        if (h) this->thread_pool::enqueue_no_future([h = ::std::move(h)] mutable { h(); });
    }

    void enqueue(generator_on_the_fly h) noexcept
    {
        if (h) this->thread_pool::enqueue_no_future([h = ::std::move(h)] mutable { h(); });
    }

    void enqueue(const per_consumer_attr& ca, task_concept auto t) noexcept
    {
        this->enqueue(ca, task_on_the_fly(t));
    }

    void enqueue(const per_consumer_attr& ca, task_on_the_fly h) noexcept
    {
        if (h) thread_pool::enqueue_no_future(ca, [h = ::std::move(h)] mutable { h(); });
    }

    void enqueue(const per_consumer_attr& ca, generator_on_the_fly h) noexcept
    {
        if (h) thread_pool::enqueue_no_future(ca, [h = ::std::move(h)] mutable { h(); });
    }

    virtual void stop() noexcept { this->thread_pool::stop(); }
    void quick_stop() noexcept { this->thread_pool::quick_stop(); }

    virtual ~task_scheduler() noexcept {}
};

KOIOS_NAMESPACE_END

#endif
