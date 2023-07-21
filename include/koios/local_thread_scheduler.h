#ifndef KOIOS_LOCAL_THREAD_SCHEDULER_H
#define KOIOS_LOCAL_THREAD_SCHEDULER_H

#include <coroutine>
#include <queue>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

class local_thread_scheduler
{
public:
    void enqueue(task_on_the_fly h)
    {
        m_tasks.emplace(::std::move(h));
        run();
    }

private:
    void run() noexcept
    {
        while (!m_tasks.empty())
        {
            auto h = ::std::move(m_tasks.front());
            m_tasks.pop();
            try { h(); } catch (...) {}
        }
    }

private:
    ::std::queue<task_on_the_fly> m_tasks;
};

KOIOS_NAMESPACE_END

#endif
