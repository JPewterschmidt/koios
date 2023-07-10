#ifndef KOIOS_LOCAL_THREAD_SCHEDULER_H
#define KOIOS_LOCAL_THREAD_SCHEDULER_H

#include <coroutine>
#include <queue>

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

class local_thread_scheduler
{
public:
    void enqueue(::std::coroutine_handle<> h)
    {
        m_tasks.emplace(h);
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
    ::std::queue<::std::coroutine_handle<>> m_tasks;
};

KOIOS_NAMESPACE_END

#endif
