#include "koios/runtime.h"
#include "koios/task_scheduler.h"

#include <utility>

KOIOS_NAMESPACE_BEG

namespace
{
    ::std::unique_ptr<task_scheduler> g_ts_p;
}

task_scheduler& get_task_scheduler()
{
    return *g_ts_p;
}

void runtime_init(size_t numthr, manually_stop_type)
{
    g_ts_p.reset(new task_scheduler{ numthr, manually_stop });
}

void runtime_init(size_t numthr)
{
    g_ts_p.reset(new task_scheduler{ numthr });
}

int runtime_exit()
{
    g_ts_p.reset(nullptr);
    return 0;
}

::std::unique_ptr<task_scheduler> 
exchange_task_scheduler(::std::unique_ptr<task_scheduler> other)
{
    return ::std::exchange(g_ts_p, ::std::move(other));
}

KOIOS_NAMESPACE_END
