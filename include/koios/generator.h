#ifndef KOIOs_GENERATOR_H
#define KOIOs_GENERATOR_H

#include <coroutine>
#include <memory>
#include <cassert>
#include <utility>
#include <stdexcept>

#include "koios/macros.h"
#include "koios/promise_base.h"
#include "koios/driver_policy.h"

KOIOS_NAMESPACE_BEG

template<typename T>
struct _generator
{
    struct [[nodiscard]] _type;
};

template<typename T>
struct generator_promise_type : promise_base
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

    decltype(auto) value()
    {
        return *m_current_value_p;
    }
};

template<typename T>
struct generator_promise_type<T&> : promise_base
{
    using handle_type = ::std::coroutine_handle<generator_promise_type<T&>>;

    T* m_current_ref{ nullptr };

    static _generator<T&>::_type get_return_object_on_allocation_failure()
    {
        return { handle_type{} };
    }

    _generator<T&>::_type get_return_object()
    {
        return { handle_type::from_promise(*this) };
    }

    constexpr void return_void() const noexcept {}

    auto yield_value(T& ref) noexcept
    {
        m_current_ref = &ref;
        return ::std::suspend_always{};
    }

    bool has_value() const noexcept
    {
        return bool(m_current_ref);
    }

    T& value()
    {
        assert(m_current_ref);
        return *m_current_ref;
    }
};

template<typename T>
class _generator<T>::_type
{
public:
    friend class generator_promise_type<T>;
    using promise_type = generator_promise_type<T>;

    bool move_next()
    {
        if (m_coro == nullptr) [[unlikely]]
            return false;
        return (m_coro.resume(), !m_coro.done());
    }

    bool has_value() const noexcept { return m_coro ? m_coro.promise().has_value() : false; }

    decltype(auto) current_value()
    {
        auto& promise = m_coro.promise();
        if (!promise.has_value()) 
            throw ::std::out_of_range{ "There's no next val could be retrived, call `has_value()` first!" };
        return ::std::move(promise.value());
    }

    _type(const _type&) = delete;
    _type(_type&& other) noexcept
        : m_coro{ ::std::exchange(other.m_coro, nullptr) }
    {
    }

    ~_type() noexcept
    {
        if (m_coro) 
        {
            m_coro.destroy();
            m_coro = nullptr;
        }
    }

private:
    _type(::std::coroutine_handle<promise_type> h) 
        : m_coro{ h } 
    {
    }

    ::std::coroutine_handle<promise_type> m_coro;
};

template<typename T>
using generator = typename _generator<T>::_type;

KOIOS_NAMESPACE_END

#endif
