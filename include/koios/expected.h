#ifndef KOIOS_EXPECTED_H
#define KOIOS_EXPECTED_H

#include "koios/macros.h"
#include "koios/functional.h"
#include "koios/expected_concepts.h"
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

    auto and_then(expected_callable_concept auto f)
        -> exp_cpt_detials::get_return_type_t<decltype(f)>
    {
        using functor_type = decltype(f);
        using functor_return_type = exp_cpt_detials::get_return_type_t<functor_type>;

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

template<typename T, typename Err, driver_policy_concept D = run_this_async>
using expected_task = typename _task<expected<T, Err>, D, discardable>::_type;

KOIOS_NAMESPACE_END

#endif
