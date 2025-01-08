// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_FUNCTIONAL_H
#define KOIOS_FUNCTIONAL_H

#include "koios/macros.h"
#include "koios/task.h"
#include "koios/task_concepts.h"
#include <concepts>
#include <tuple>

KOIOS_NAMESPACE_BEG

auto identity(auto arg) -> task<decltype(arg)>
{
    co_return arg;
}

template<typename Func, typename... Args>
requires (
    task_callable_concept<Func> 
    and !lazy_task_callable_concept<Func>)
auto make_lazy(Func f, Args... args) 
    -> lazy_task<typename toolpex::get_return_type_t<Func>::value_type>
{
    // It is a best practice to co_return a co_await'ed result in make_lazy coroutine.
    co_return co_await f(::std::move(args)...);
}

template<typename Func, typename... Args>
requires (lazy_task_callable_concept<Func>)
auto make_lazy(Func f, Args... args)
    -> lazy_task<typename toolpex::get_return_type_t<Func>::value_type>
{
    return f(::std::move(args)...);
}

template<awaitible_concept... Aws>
auto co_await_all(Aws... aws)
    -> task<::std::tuple<awaitable_result_type_t<Aws>...>>
{
    co_return ::std::make_tuple((co_await ::std::move(aws))...);
}

KOIOS_NAMESPACE_END

#endif
