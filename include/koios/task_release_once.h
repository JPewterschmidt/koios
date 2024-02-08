#ifndef KOIOS_TASK_CALL_ONCE_H
#define KOIOS_TASK_CALL_ONCE_H

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include <memory>
#include <atomic>
#include <optional>

KOIOS_NAMESPACE_BEG

class task_release_once
{
public:
    task_release_once(task_on_the_fly t)
        : m_task{ ::std::move(t) }
    {
    }

    ::std::optional<task_on_the_fly> release()
    {
        bool expected{ true };
        if (m_valid.compare_exchange_strong(expected, false))
        {
            return ::std::move(m_task);
        }
        return {};
    }
    
private:
    ::std::atomic_bool m_valid{true};
    task_on_the_fly m_task;
};

KOIOS_NAMESPACE_END

#endif
