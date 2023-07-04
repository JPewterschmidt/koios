#ifndef KOIOS_LOCAL_THREAD_SCHEDULER_H
#define KOIOS_LOCAL_THREAD_SCHEDULER_H

#include <coroutine>
#include <queue>

#include "koios/macros.h"
#include "koios/tiny_task.h"

KOIOS_NAMESPACE_BEG

class local_thread_scheduler
{
public:
    void enqueue(::std::coroutine_handle<> h)
    {
        m_tasks.emplace(h);
    }

    void run() noexcept
    {
        while (!m_tasks.empty())
        {
            tiny_task t = ::std::move(m_tasks.top());
            m_tasks.pop();
            try { t(); } catch (...) {}
        }
    }

private:
    ::std::queue<tiny_task> m_tasks;
};

KOIOS_NAMESPACE_END

#endif
