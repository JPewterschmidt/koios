#ifndef KOIOS_GENERATOR_H
#define KOIOS_GENERATOR_H

#include <coroutine>
#include <memory>
#include <cassert>
#include <utility>
#include <stdexcept>

#include "koios/macros.h"
#include "koios/promise_base.h"
#include "koios/driver_policy.h"
#include "koios/generator_iterator.h"

KOIOS_NAMESPACE_BEG

template<typename T>
struct _generator
{
    struct [[nodiscard]] _type;
};

template<typename T>
struct generator_promise_type : promise_base<::std::suspend_always>
{
    using handle_type = ::std::coroutine_handle<generator_promise_type<T>>;

    ::std::unique_ptr<T> m_current_value_p{ nullptr };

    static _generator<T>::_type get_return_object_on_allocation_failure() 
    { 
        return { handle_type{} }; 
    }

    _generator<T>::_type get_return_object() 
    { 
        return { handle_type::from_promise(*this) }; 
    }

    constexpr void return_void() const noexcept {}

    template<typename TT>
    auto yield_value(TT&& val)
    {
        m_current_value_p.reset(new T(::std::forward<TT>(val)));
        return ::std::suspend_always{};
    }

    bool has_value() const noexcept
    {
        return bool(m_current_value_p);
    }

    T value()
    {
        auto resultp = ::std::move(m_current_value_p);
        return ::std::move(*resultp);
    }

    auto& value_storage() noexcept { return m_current_value_p; }

    void clear() noexcept { m_current_value_p.reset(); }
};

template<typename T>
class _generator<T>::_type
{
    static_assert(!::std::is_reference_v<T>, "The `result_type` of generator could not be a reference!");
public:
    friend class generator_promise_type<T>;
    using promise_type = generator_promise_type<T>;
    using result_type = T;

    bool move_next()
    {
        if (m_coro == nullptr)
            return false;
        m_coro.promise().clear();
        return (m_coro.resume(), !m_coro.done());
    }

    bool has_value() const noexcept { return m_coro ? m_coro.promise().has_value() : false; }

    decltype(auto) current_value()
    {
        auto& promise = m_coro.promise();
        if (!promise.has_value()) 
        {
            throw ::std::out_of_range{ 
                "There's no next val could be retrived, call `has_value()` first! "
                "Or, you may perform co_await call, it's not supported yet."
            };
        }
        return promise.value();
    }

    auto& current_value_storage()
    {
        return m_coro.promise().value_storage();
    }

    _type(const _type&) = delete;
    _type(_type&& other) noexcept
        : m_coro{ ::std::exchange(other.m_coro, nullptr) }, 
          m_need_destroy_in_dtor{ ::std::exchange(other.m_need_destroy_in_dtor, false) }
    {
    }

    _type& operator = (_type&& other) noexcept
    {
        destroy_current_coro();

        m_coro = ::std::exchange(other.m_coro, nullptr);
        m_need_destroy_in_dtor = ::std::exchange(other.m_need_destroy_in_dtor, false);

        return *this;
    }

    ~_type() noexcept { destroy_current_coro(); }

private:
    _type(::std::coroutine_handle<promise_type> h) 
        : m_coro{ h } 
    {
    }

    void destroy_current_coro() noexcept
    {
        if (::std::exchange(m_need_destroy_in_dtor, false)) [[likely]]
            m_coro.destroy();
    }

    ::std::coroutine_handle<promise_type> m_coro;
    bool m_need_destroy_in_dtor{ true };

public:
    using iterator = detial::generator_iterator<_generator<T>::_type>;
    iterator begin() noexcept { return { *this }; }
    constexpr detial::generator_iterator_sentinel end() const noexcept { return {}; };
};


template<typename T>
using generator = typename _generator<T>::_type;

KOIOS_NAMESPACE_END

#endif
