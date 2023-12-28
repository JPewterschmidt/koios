#pragma once
#ifndef KOIOS_RUNTIME_H
#define KOIOS_RUNTIME_H

#include "koios/macros.h"
#include "koios/task_scheduler.h"
#include "koios/task_scheduler_concept.h"
#include "koios/event_loop.h"
#include "koios/timer.h"
#include "koios/iouring.h"

#include <memory>
#include <source_location>

KOIOS_NAMESPACE_BEG

using event_loop_t = event_loop<
    timer_event_loop, 
    iouring_event_loop
>;

::std::unique_ptr<task_scheduler>& 
get_task_scheduler_ptr(::std::source_location sl);

inline 
task_scheduler_concept 
auto& get_task_scheduler(
    ::std::source_location sl = ::std::source_location::current())
{
    return static_cast<event_loop_t&>(
        *get_task_scheduler_ptr(::std::move(sl))
    );
}

void runtime_init(size_t numthr);
void runtime_init(size_t numthr, manually_stop_type);
int runtime_exit();

void runtime_reload(size_t numthr);
void runtime_reload(size_t numthr, manually_stop_type);

::std::unique_ptr<task_scheduler> 
exchange_task_scheduler(::std::unique_ptr<task_scheduler> other);

KOIOS_NAMESPACE_END

#endif
