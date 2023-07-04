#ifndef KOIOS_TASK_SCHEDULER_SELECTOR_H
#define KOIOS_TASK_SCHEDULER_SELECTOR_H

#include <coroutine>

#include "koios/macros.h"
#include "koios/global_task_scheduler.h"
#include "koios/task_scheduler_wrapper.h"
#include "koios/local_thread_scheduler.h"

KOIOS_NAMESPACE_BEG

class task_scheduler_selector
{
public:
    consteval bool need_run_sync() const noexcept { return false; }

    task_scheduler_wrapper scheduler() const noexcept 
    {
        return get_task_scheduler();
    }
};

KOIOS_NAMESPACE_END

#endif
