// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_LOCAL_THREAD_SCHEDULER_H
#define KOIOS_LOCAL_THREAD_SCHEDULER_H

#include <coroutine>
#include <queue>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

/*! \brief A tiny task scheduler which allow tasks run synchronously. */ 
class local_thread_scheduler
{
public:
    /*! This is sync call, which means only the scheduled task return, this function return.
     *  Of course, this will call the coroutine immediately.
     */
    void enqueue(task_on_the_fly h)
    {
        m_tasks.emplace(::std::move(h));
        this->run();
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
    // std::queue is not suppose to be there...
    ::std::queue<task_on_the_fly> m_tasks;
};

KOIOS_NAMESPACE_END

#endif
