#ifndef KOIOS_DRIVER_POLICY_H
#define KOIOS_DRIVER_POLICY_H

#include "koios/macros.h"
#include "koios/task_scheduler_concept.h"
#include "koios/local_thread_scheduler.h"
#include "koios/global_task_scheduler.h"
#include "koios/task_scheduler_wrapper.h"

KOIOS_NAMESPACE_BEG

template<typename DP>
concept driver_policy_concept = requires(DP dp)
{
    { dp.scheduler() } -> task_scheduler_concept;
};

struct run_this_async
{
    task_scheduler_wrapper scheduler()
    {
        return { get_task_scheduler() };
    };
};

struct run_this_sync
{
    task_scheduler_owned_wrapper scheduler()
    {
        return { local_thread_scheduler{} };
    };
};

KOIOS_NAMESPACE_END

#endif
