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
#include <atomic>

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

/**
 *  \brief emit ranges::distance(r) threads to map the element of the range by coroutine `t` parallely.
 *  \param r a range which contains a limited numbner of element which going to be the first argument of coroutine `t`
 *  \param t a coroutine callable that receive the element of range `r` as the first argument, and `args...` as other argument.
 *  \param args... arguments that will be feed to `t` follows the first argument (a element of the range).
 *
 *  \attention make sure the range could be drained, or there will be infinity thread be emitted.
 */
template<typename... Args>
task<> for_each(::std::ranges::range auto&& r, task_callable_concept auto t, Args&&... args)
{
    namespace rv = ::std::ranges::views;

    auto args_here = ::std::forward_as_tuple(::std::forward<Args>(args)...);

    auto aws = ::std::forward<decltype(r)>(r) | rv::transform([&](auto&& item) mutable { 
        auto packed = ::std::tuple_cat(::std::forward_as_tuple(::std::forward<decltype(item)>(item)), args_here);
        return ::std::apply([&](auto&& item, auto&&... args) mutable { 
            return make_lazy(t, ::std::forward<decltype(item)>(item), ::std::forward<decltype(args)>(args)...).run_and_get_future();
        }, ::std::move(packed));
    });
    co_await co_await_all(aws);
}

/**
 *  \brief emit ranges::distance(r) threads to map the element of the range by coroutine `t` parallely.
 *  \param r a range which contains a limited numbner of element which going to be the first argument of coroutine `t`
 *  \param t a coroutine callable that receive the element of range `r` as the first argument, and `args...` as other argument.
 *  \param args... arguments that will be feed to `t` follows the first argument (a element of the range).
 *
 *  \attention make sure the range could be drained, or there will be infinity thread be emitted.
 *
 *  This function basically the same version of the previous one, 
 *  but this will try to distribute the coroutine calls on all the threads evenly.
 */
template<typename... Args>
task<> for_each_dispatch_evenly(::std::ranges::range auto&& r, task_callable_concept auto t, Args&&... args)
{
    namespace rv = ::std::ranges::views;

    static ::std::atomic_size_t dispatcher{};
    const auto& thr_attrs = get_task_scheduler().consumer_attrs();

    auto args_here = ::std::forward_as_tuple(::std::forward<Args>(args)...);

    auto aws = ::std::forward<decltype(r)>(r) | rv::transform([&](auto&& item) mutable { 
        auto packed = ::std::tuple_cat(::std::forward_as_tuple(::std::forward<decltype(item)>(item)), args_here);
        return ::std::apply([&](auto&& item, auto&&... args) mutable { 
            return make_lazy(t, ::std::forward<decltype(item)>(item), ::std::forward<Args>(args)...)
                .run_and_get_future(
                    *thr_attrs[dispatcher.fetch_add(1, ::std::memory_order_relaxed) % thr_attrs.size()]
                );
        }, ::std::move(packed));
    });
    co_await co_await_all(aws);
}

task<> for_each_int(int beg, int end, task_callable_concept auto t, auto&&... args)
{
    namespace rv = ::std::ranges::views;
    return for_each(rv::iota(beg, end), ::std::move(t), ::std::forward<decltype(args)>(args)...);
}

task<> for_each_int_dispatch_evenly(int beg, int end, task_callable_concept auto t, auto&&... args)
{
    namespace rv = ::std::ranges::views;
    return for_each_dispatch_evenly(rv::iota(beg, end), ::std::move(t), ::std::forward<decltype(args)>(args)...);
}

KOIOS_NAMESPACE_END

#endif
