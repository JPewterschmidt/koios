#ifndef KOIOS_TASK_SCHEDULER_H
#define KOIOS_TASK_SCHEDULER_H

#include <coroutine>
#include <concepts>
#include "koios/macros.h"
#include "koios/task_concepts.h"
#include "koios/thread_pool.h"

KOIOS_NAMESPACE_BEG

class task_scheduler : public thread_pool
{
public:
    explicit task_scheduler(size_t thr_cnt, manually_stop_type)
        : thread_pool{ thr_cnt, manually_stop }
    {
    }

    explicit task_scheduler(size_t thr_cnt)
        : thread_pool{ thr_cnt }
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
            tiny_task tt{ h };
            thread_pool::enqueue([tt = ::std::move(tt)]() noexcept { 
                try 
                { 
                    tt(); 
                } 
                catch (...) {}
            });
        }
    }

    void stop() noexcept { thread_pool::stop(); }
    void quick_stop() noexcept { thread_pool::quick_stop(); }

private:
    class tiny_task
    {
    public:
        tiny_task(::std::coroutine_handle<> h) noexcept
            : m_h{ ::std::move(h) }
        {
        }

        ~tiny_task() noexcept
        {
            if (m_h != nullptr && m_h.done()) 
                m_h.destroy();
        }

        tiny_task(tiny_task&& other)
            : m_h{ ::std::exchange(other.m_h, nullptr) }
        {
        }

        void operator()() const { if (m_h != nullptr) [[likely]] m_h(); }

    private:
        ::std::coroutine_handle<> m_h;
    };
};

KOIOS_NAMESPACE_END

#endif
