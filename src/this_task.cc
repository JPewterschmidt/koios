#include "koios/this_task.h"
#include "koios/runtime.h"
#include <thread>

namespace koios::this_task
{
    
task<> turn_into_scheduler()
{
    const auto tid = ::std::this_thread::get_id();
    for (const auto* attr : get_task_scheduler().consumer_attrs())
    {
        if (attr->thread_id == tid) co_return;
    }
    co_await this_task::yield();
}


} // namespace koios
