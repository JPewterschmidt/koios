#ifndef KOIOS_TASK_CONCEPTS_H
#define KOIOS_TASK_CONCEPTS_H

#include <concepts>
#include <coroutine>

#include "koios/macros.h"
#include "toolpex/concepts_and_traits.h"
#include "koios/task_on_the_fly.h"
#include "koios/future.h"

KOIOS_NAMESPACE_BEG

template<typename Aw>
concept awaitible_concept = requires(Aw a)
{
    { a.await_ready() } -> ::std::same_as<bool>;
    { a.await_suspend(::std::declval<::std::coroutine_handle<>>()) };
};

template<typename T>
concept task_concept = requires(T t)
{
    typename T::value_type;
    { static_cast<task_on_the_fly>(t) };
    { t() };
    { t.run_and_get_future() } -> toolpex::is_specialization_of<::koios::future>;
} and awaitible_concept<T>;

template<typename Func>
concept task_callable_concept = task_concept<toolpex::get_return_type_t<Func>>;

KOIOS_NAMESPACE_END

#endif
