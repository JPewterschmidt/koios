#ifndef KOIOS_DRIVER_POLICY_H
#define KOIOS_DRIVER_POLICY_H

#include "koios/macros.h"
#include "koios/task_scheduler_concept.h"
#include "koios/local_thread_scheduler.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_wrapper.h"

KOIOS_NAMESPACE_BEG

template<typename DP>
concept driver_policy_concept = requires(DP dp)
{
    { dp.scheduler() } -> task_scheduler_concept;
};

/*! \brief One of the drive policy, make the `task` runs asynchronous. */
struct run_this_async
{
    /*! \return A global asynchronous task scheduler. */
    task_scheduler_wrapper scheduler() noexcept
    {
        return { get_task_scheduler() };
    };
};

/*! \brief One of the drive policy, make the `task` runs synchronous. */
struct run_this_sync
{
    /*! \return A local synchronous task scheduler. */
    task_scheduler_owned_wrapper scheduler()
    {
        return { local_thread_scheduler{} };
    };
};

KOIOS_NAMESPACE_END

#endif
