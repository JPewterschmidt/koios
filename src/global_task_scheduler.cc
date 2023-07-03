#include "koios/global_task_scheduler.h"
#include "koios/task_scheduler.h"

KOIOS_NAMESPACE_BEG

namespace
{
    //task_scheduler g_ts{ 1, manually_stop };
    task_scheduler g_ts{ 1 };
}

task_scheduler& get_task_scheduler()
{
    return g_ts;
}

KOIOS_NAMESPACE_END
