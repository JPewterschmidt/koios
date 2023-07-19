#include "koios/runtime.h"
#include "koios/task_scheduler.h"
#include "spdlog/spdlog.h"

#include <utility>

KOIOS_NAMESPACE_BEG

namespace
{
    ::std::unique_ptr<task_scheduler> g_ts_p;

    void logging_init()
    {
        spdlog::set_level(spdlog::level::info);
    }
}

task_scheduler& get_task_scheduler(::std::source_location sl)
{
    if (!g_ts_p) [[unlikely]]
        throw runtime_not_working_exception{ ::std::move(sl) };
    return *g_ts_p;
}

void runtime_init(size_t numthr, manually_stop_type)
{
    g_ts_p.reset(new task_scheduler{ numthr, manually_stop });
    logging_init();
}

void runtime_init(size_t numthr)
{
    g_ts_p.reset(new task_scheduler{ numthr });
    logging_init();
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
