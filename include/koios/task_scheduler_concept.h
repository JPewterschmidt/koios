#ifndef KOIOS_TASK_SCHEDULER_CONCEPT_H
#define KOIOS_TASK_SCHEDULER_CONCEPT_H

#include <concepts>
#include <coroutine>
#include <type_traits>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"

KOIOS_NAMESPACE_BEG

template<typename TS>
concept task_scheduler_concept = requires(TS ts)
{
    //{ ts.enqueue(::std::declval<::std::coroutine_handle<>>()) };
    { ts.enqueue(::std::declval<koios::task_on_the_fly>()) };
};

template<typename TS>
concept task_scheduler_wrapper_concept = task_scheduler_concept<TS> && requires(TS ts)
{
    { typename TS::is_wrapper_t{} } -> ::std::same_as<::std::true_type>;
};

KOIOS_NAMESPACE_END

#endif
