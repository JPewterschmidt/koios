// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_TASK_CONCEPTS_H
#define KOIOS_TASK_CONCEPTS_H

#include <concepts>
#include <coroutine>
#include <type_traits>

#include "koios/macros.h"
#include "toolpex/concepts_and_traits.h"
#include "koios/task_on_the_fly.h"
#include "koios/future.h"

KOIOS_NAMESPACE_BEG

template<typename Aw>
concept pure_awaitible_concept = requires(Aw a)
{
    { a.await_ready() } -> toolpex::boolean_testable;
    { a.await_suspend(::std::declval<::std::coroutine_handle<>>()) };
    a.await_resume();
};

template<typename Aw>
concept convertible_awaitible_concept = requires(Aw a)
{
    { a.operator co_await() } -> pure_awaitible_concept;
};

template<typename Aw>
concept awaitible_concept = pure_awaitible_concept<Aw> or convertible_awaitible_concept<Aw>;

template<typename Aw>
struct pure_awaitable_result_type
{
    using type = decltype(::std::declval<Aw>().await_resume());
};

template<typename Aw>
struct convertible_awaitable_result_type
{
    using type = decltype(::std::declval<Aw>().operator co_await().await_resume());
};

template<awaitible_concept Aw>
struct awaitable_result_type
{
private:
    using cond_type = ::std::conditional_t<
        pure_awaitible_concept<Aw>, 
        pure_awaitable_result_type<Aw>, 
        convertible_awaitable_result_type<Aw>
    >;

public:
    using type = typename cond_type::type;
};

template<typename Aw>
using awaitable_result_type_t = awaitable_result_type<Aw>::type;

template<typename Func>
concept awaitable_callable_concept = awaitible_concept<toolpex::get_return_type_t<Func>>;

template<typename T>
concept task_concept = requires(T t)
{
    typename T::value_type;
    { static_cast<task_on_the_fly>(t) };
    { t() };
    { t.run_and_get_future() } -> toolpex::is_specialization_of<::koios::future>;
} and awaitible_concept<T>;

template<typename T>
concept lazy_task_concept = 
    task_concept<T> 
    and ::std::same_as<typename T::initial_suspend_type, ::std::suspend_always>;

template<typename Func>
concept task_callable_concept = task_concept<toolpex::get_return_type_t<Func>>;

template<typename Func, typename Ret>
concept task_callable_with_result_concept = 
    task_callable_concept<Func> && 
    ::std::same_as<
        awaitable_result_type_t<toolpex::get_return_type_t<Func>>, 
        Ret>;

template<typename Func>
concept lazy_task_callable_concept = 
    task_callable_concept<Func> 
    and lazy_task_concept<toolpex::get_return_type_t<Func>>;

template <typename...>
struct is_all_same_type 
{
    static constexpr bool value = false;
};

template <typename T, typename U, typename... Rest>
struct is_all_same_type<T, U, Rest...>
{
    static constexpr bool value = false;
};

template <typename T>
struct is_all_same_type<T> 
{
    static constexpr bool value = true;
};

template <typename T>
struct is_all_same_type<T, T> 
{
    static constexpr bool value = true;
};

template <typename T>
struct is_all_same_type<T, T, T> 
{
    static constexpr bool value = true;
};

template <typename T, typename U, typename... Rest>
struct is_all_same_type<T, U, T, Rest...> 
{
    static constexpr bool value = is_all_same_type<T, U>::value && is_all_same_type<T, Rest...>::value;
};

template <typename T, typename U, typename... Rest>
struct is_all_same_type<T, U, U, Rest...> 
{
    static constexpr bool value = is_all_same_type<T, U>::value && is_all_same_type<U, Rest...>::value;
};

template<typename... T>
inline constexpr bool is_all_same_type_v = is_all_same_type<T...>::value;

template <typename T, typename... Rest>
struct first_type
{
    using type = T;
};

template <typename T>
struct first_type<T>
{
    using type = T;
};

template <typename... Ts>
using first_type_t = typename first_type<Ts...>::type;

template <awaitible_concept... Aws>
struct is_all_same_result_aws
{
    static constexpr bool value = is_all_same_type_v<awaitable_result_type_t<Aws>...>;
    using type = typename ::std::enable_if<value, first_type_t<awaitable_result_type_t<Aws>...>>::type;
};

template <awaitible_concept... Aws>
inline constexpr bool is_all_same_result_aws_v = is_all_same_result_aws<Aws...>::value;

KOIOS_NAMESPACE_END

#endif
