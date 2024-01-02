#ifndef KOIOS_TASK_CONCEPTS_H
#define KOIOS_TASK_CONCEPTS_H

#include <concepts>
#include <coroutine>

#include "koios/macros.h"
#include "toolpex/is_specialization_of.h"
#include "koios/task_on_the_fly.h"
#include "koios/future.h"

KOIOS_NAMESPACE_BEG

template<typename T>
concept task_concept = requires(T t)
{
    //{ t.get_handler_to_schedule() } -> ::std::convertible_to<::std::coroutine_handle<>>;
    { static_cast<task_on_the_fly>(t) };
    { t() };
    { t.run_and_get_future() } -> toolpex::is_specialization_of<::koios::future>;
};

KOIOS_NAMESPACE_END

#endif
