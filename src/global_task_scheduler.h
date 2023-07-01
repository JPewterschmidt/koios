#include "koios/global_task_scheduler.h"
#include "koios/task_scheduler.h"

task_scheduler g_ts{ 10 };

task_scheduler& get_task_scheduler()
{
    return g_ts;
}
