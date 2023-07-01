#ifndef KOIOS_GLOBAL_TASK_SCHEDULER_H
#define KOIOS_GLOBAL_TASK_SCHEDULER_H

#include "koios/macros.h"
#include "koios/task_scheduler.h"

KOIOS_NAMESPACE_BEG

task_scheduler& get_task_scheduler();

KOIOS_NAMESPACE_END

#endif
