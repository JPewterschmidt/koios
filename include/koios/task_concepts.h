#ifndef KOIOS_TASK_CONCEPTS_H
#define KOIOS_TASK_CONCEPTS_H

#include <concepts>
#include <coroutine>

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

template<typename T>
concept task_concept = requires(T t)
{
    { t.move_out_coro_handle() } -> ::std::convertible_to<::std::coroutine_handle<>>;
    { t() };
    { t.get_future() };
};

KOIOS_NAMESPACE_END

#endif
