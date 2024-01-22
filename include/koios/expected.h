/* Koios, A c++ async runtime library.
 * Copyright (C) 2023  Jeremy Pewterschmidt
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

#ifndef KOIOS_EXPECTED_H
#define KOIOS_EXPECTED_H

#include "koios/macros.h"
#include "koios/functional.h"
#include "koios/expected_concepts.h"
#include "koios/task_concepts.h"
#include "toolpex/concepts_and_traits.h"
#include <variant>

KOIOS_NAMESPACE_BEG

template<typename Err>
requires(::std::is_nothrow_move_constructible_v<Err> 
     and ::std::is_nothrow_move_assignable_v<Err>)
class unexpected_t;

constexpr enum class unexpected_tag_t{} unexpected_tag{};

template<typename T, typename Err>
requires (!toolpex::is_specialization_of<T, unexpected_t>)
class expected
{
public:
    using value_type = T;
    using error_type = Err;

public:
    constexpr expected(value_type val)
        : m_storage(::std::in_place_index_t<0>{}, ::std::move(val))
    {
    }

    template<typename... Args>
    expected(unexpected_tag_t, Args&&... args) 
        noexcept(::std::is_nothrow_constructible_v<Err>)
        : m_storage(::std::in_place_index_t<1>{}, ::std::forward<Args>(args)...)
    {
    }

    constexpr bool has_value() const noexcept { return m_storage.index() == 0; }
    value_type& value() noexcept { return get<0>(m_storage); }
    error_type& error() noexcept { return get<1>(m_storage); }

    operator value_type&() 
    { 
        if (has_value()) [[likely]] return value(); 
        throw koios::expected_exception{KOIOS_EXPECTED_NOTHING_TO_GET};
    }

    auto and_then(expected_callable_concept auto f)
        -> toolpex::get_return_type_t<decltype(f)>
    {
        using functor_type = decltype(f);
        using functor_return_type = toolpex::get_return_type_t<functor_type>;

        if (has_value())
        {
            return f(::std::move(value()));
        }

        if constexpr (regular_expected_like_concept<functor_return_type>)
        {
            return {unexpected_tag, ::std::move(error())};
        }
        else if constexpr (expected_like_astask_concept<functor_return_type>)
        {
            return koios::identity(typename functor_return_type::value_type{
                unexpected_tag, ::std::move(error())
            });
        }
        else toolpex::not_implemented();
    }

private:
    ::std::variant<value_type, error_type> m_storage;
};

template<typename Err>
requires(::std::is_nothrow_move_constructible_v<Err> 
     and ::std::is_nothrow_move_assignable_v<Err>)
class unexpected_t
{
public:
    using error_type = Err;

    template<typename Arg>
    constexpr unexpected_t(Arg&& args) noexcept
        : m_err(::std::forward<Arg>(args))
    {
    }

    unexpected_t(unexpected_t&&) noexcept = default;
    unexpected_t& operator=(unexpected_t&&) noexcept = default;

    error_type& error() noexcept { return m_err; }

    template<typename T>
    operator expected<T, error_type>()
    {
        return { unexpected_tag, ::std::move(error()) };
    }

private:
    error_type m_err;
};

auto unexpected(auto&& arg)
{
    return unexpected_t<::std::remove_reference_t<decltype(arg)>>{ 
        ::std::forward<decltype(arg)>(arg)
    };
}

template<typename T, typename Err, driver_policy_concept D = run_this_async, typename InitialSuspendAw = eager_aw>
using expected_task = typename _task<expected<T, Err>, D, discardable, InitialSuspendAw>::_type;

template<typename T, typename Err, driver_policy_concept D = run_this_async, typename InitialSuspendAw = ::std::suspend_always>
using emitter_expected_task = typename _task<expected<T, Err>, D, discardable, InitialSuspendAw>::_type;

template<typename T = void>
using exp_taskec = expected_task<T, ::std::error_code>;

KOIOS_NAMESPACE_END

#endif
