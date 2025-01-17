// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_FUNCTIONAL_H
#define KOIOS_FUNCTIONAL_H

#include "koios/macros.h"
#include "koios/task.h"
#include "koios/task_concepts.h"
#include "koios/runtime.h"
#include <concepts>
#include <tuple>
#include <ranges>
#include <vector>

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
auto make_lazy(Func f, Args&&... args)
    -> lazy_task<typename toolpex::get_return_type_t<Func>::value_type>
{
    return f(::std::forward<Args>(args)...);
}

template<awaitible_concept... Aws>
requires ((!::std::same_as<awaitable_result_type_t<Aws>, void>) and ...)
auto co_await_all(Aws... aws)
    -> task<::std::tuple<awaitable_result_type_t<Aws>...>>
{
    co_return ::std::make_tuple((co_await ::std::move(aws))...);
}

template<awaitible_concept... Aws>
requires (::std::same_as<awaitable_result_type_t<Aws>, void> and ...)
auto co_await_all(Aws... aws)
    -> task<::std::tuple<awaitable_result_type_t<Aws>...>>
{
    ((co_await ::std::move(aws)), ...);
}

template<typename Aws>
requires (::std::ranges::range<Aws> 
      and awaitible_concept<::std::ranges::range_value_t<Aws>>
      and !::std::same_as<void, awaitable_result_type_t<::std::ranges::range_value_t<Aws>>>)
auto co_await_all(Aws aws)
    -> task<::std::vector<awaitable_result_type_t<::std::ranges::range_value_t<Aws>>>>
{
    ::std::vector<awaitable_result_type_t<::std::ranges::range_value_t<Aws>>> result;
    for (auto&& aw : aws)
    {
        result.push_back(co_await ::std::forward<decltype(aw)>(aw));
    }
    co_return result;
}

template<typename Aws>
requires (::std::ranges::range<Aws> 
      and awaitible_concept<::std::ranges::range_value_t<Aws>>
      and ::std::same_as<void, awaitable_result_type_t<::std::ranges::range_value_t<Aws>>>)
auto co_await_all(Aws aws) -> task<>
{
    for (auto&& aw : aws)
    {
        co_await ::std::forward<decltype(aw)>(aw);
    }
}

template<typename... Args>
task<> for_each(::std::ranges::range auto&& r, task_callable_concept auto t, Args&&... args)
{
    ::std::vector<koios::future<void>> futs;
    for (auto&& item : ::std::forward<decltype(r)>(r))
    {
        futs.push_back(make_lazy(
            t, 
            ::std::forward<decltype(item)>(item), 
            ::std::forward<Args>(args)...
        ).run_and_get_future());
    }
    co_await co_await_all(::std::move(futs));
}

template<typename... Args>
task<> for_each_dispatch_evenly(::std::ranges::range auto&& r, task_callable_concept auto t, Args&&... args)
{
    ::std::vector<koios::future<void>> futs;
    static size_t dispatcher{};
    const auto& thr_attrs = get_task_scheduler().consumer_attrs();
    for (auto&& item : ::std::forward<decltype(r)>(r))
    {
        futs.push_back(make_lazy(
            t, 
            ::std::forward<decltype(item)>(item), 
            ::std::forward<Args>(args)...
        ).run_and_get_future_on(thr_attrs[dispatcher++ % thr_attrs.size()]));
    }
    co_await co_await_all(::std::move(futs));
}

KOIOS_NAMESPACE_END

#endif
