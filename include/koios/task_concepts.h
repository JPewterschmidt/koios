/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
concept awaitible_concept = requires(Aw a)
{
    { a.await_ready() } -> ::std::same_as<bool>;
    { a.await_suspend(::std::declval<::std::coroutine_handle<>>()) };
    a.await_resume();
};

template<awaitible_concept Aw>
struct awaitable_result_type
{
    using type = decltype(::std::declval<Aw>().await_resume());
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
concept eager_task_concept = 
    task_concept<T> 
    and ::std::same_as<typename T::initial_suspend_type, ::std::suspend_always>;

template<typename Func>
concept task_callable_concept = task_concept<toolpex::get_return_type_t<::std::remove_reference_t<Func>>>;

template<typename Func>
concept eager_task_callable_concept = 
    task_callable_concept<Func> 
    and eager_task_concept<toolpex::get_return_type_t<Func>>;

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
