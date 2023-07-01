#ifndef KOIOS_TASK_SCHEDULER_CONCEPT_H
#define KOIOS_TASK_SCHEDULER_CONCEPT_H

#include <concepts>
#include <coroutine>

template<typename TS>
concept task_scheduler_concept = requires(TS ts)
{
    { ts.enqueue(::std::declval<::std::coroutine_handle<>>()) };
};


#endif
