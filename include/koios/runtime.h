#pragma once
#ifndef KOIOS_RUNTIME_H
#define KOIOS_RUNTIME_H

#include "koios/macros.h"
#include "koios/task_scheduler.h"
#include "koios/task_scheduler_concept.h"

#include <memory>
#include <source_location>

KOIOS_NAMESPACE_BEG

task_scheduler& get_task_scheduler_impl(::std::source_location sl);

inline 
task_scheduler_concept 
auto& get_task_scheduler(
    ::std::source_location sl = ::std::source_location::current())
{
    return get_task_scheduler_impl(::std::move(sl));
}

void runtime_init(size_t numthr);
void runtime_init(size_t numthr, manually_stop_type);
int runtime_exit();

void runtime_reload(size_t numthr);
void runtime_reload(size_t numthr, manually_stop_type);

::std::unique_ptr<task_scheduler> exchange_task_scheduler(::std::unique_ptr<task_scheduler> other);

KOIOS_NAMESPACE_END

#endif
