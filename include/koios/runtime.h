#ifndef KOIOS_GLOBAL_TASK_SCHEDULER_H
#define KOIOS_GLOBAL_TASK_SCHEDULER_H

#include "koios/macros.h"
#include "koios/task_scheduler.h"

#include <memory>

KOIOS_NAMESPACE_BEG

task_scheduler& get_task_scheduler();

void runtime_init(size_t numthr);
void runtime_init(size_t numthr, manually_stop_type);
int runtime_exit();

::std::unique_ptr<task_scheduler> exchange_task_scheduler(::std::unique_ptr<task_scheduler> other);

KOIOS_NAMESPACE_END

#endif
