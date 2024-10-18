// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_GENERATOR_H
#define KOIOS_GENERATOR_H

#include <coroutine>
#include <memory>
#include <utility>
#include <stdexcept>
#include <memory>
#include <mutex>
#include <functional>

#include "koios/macros.h"
#include "koios/promise_base.h"
#include "koios/generator_iterator.h"
#include "koios/generator_concepts.h"
#include "koios/task_on_the_fly.h"
#include "koios/waiting_handle.h"
#include "koios/task.h"

KOIOS_NAMESPACE_BEG

template<typename T, typename Alloc>
struct _generator
{
    struct [[nodiscard]] _type;
};

/*! \brief The `generator` specific promise class.
 *  The ownership of handler won't move out.
 *
 *  \tparam Alloc The allocator type used.
 */
template<typename T, typename Alloc>
class generator_promise_type 
    : public promise_base<lazy_aw, ::std::suspend_always>
{
    size_t m_debug_canary{888};
public:
    /*! The deleter for `::std::unique_ptr`, which firstly destruct the object on it,
     *  the deallocate it by the allocator.
     */
    struct value_deleter
    {
        void operator()(T* p) const noexcept 
        {
            p->~T();
            Alloc{}.deallocate(p, 1);
        }
    };

    using handle_type = ::std::coroutine_handle<generator_promise_type<T, Alloc>>;

    ~generator_promise_type() noexcept
    {
        toolpex_assert(!m_waitting);
        m_debug_canary = 1666;
    }

private:
    mutable ::std::mutex m_mutex;
    ::std::byte m_value_storage[sizeof(T)];
    bool m_has_val{};
    task_on_the_fly m_waitting{};
    bool m_finalized{};

public:
    class get_yielded_aw
    {
        size_t m_debug_canary{};
        bool m_debug_after_ready{};
        bool m_debug_after_resume{};
    public:
        get_yielded_aw(::std::coroutine_handle<generator_promise_type> h) noexcept
            : m_h{ h }, m_parent{ h.promise() }
        {
        }

        ~get_yielded_aw() noexcept 
        { 
            toolpex_assert(m_debug_after_resume);
            toolpex_assert(m_debug_after_ready);
            m_debug_canary = 0x8888; 
        }

        bool await_ready() noexcept
        {
            toolpex_assert(!m_debug_after_ready);
            m_debug_after_ready = true;
            m_parent.m_mutex.lock();
            if (m_parent.finalized_impl())
            {
                m_without_suspend = true;
                return true;
            }
            else if (!m_parent.has_value_impl())
            {
                wake_up(::std::move(m_h));
                return false;
            }

            return m_parent.has_value_impl();
        }

        void await_suspend(task_on_the_fly t) noexcept
        {
            toolpex_assert(!m_parent.m_waitting);
            toolpex_assert(!m_parent.m_mutex.try_lock());
            m_parent.m_waitting = ::std::move(t);
            m_parent.m_mutex.unlock();
        }

        ::std::optional<T> await_resume() 
        {
            toolpex_assert(!m_debug_after_resume);
            if (m_without_suspend)
            {
                m_parent.m_mutex.lock();
            }
            auto ret = m_parent.value_opt_impl();
            m_debug_after_resume = true;

            return ret;
        }

    private:
        generator_on_the_fly m_h{}; 
        generator_promise_type& m_parent;

        bool m_without_suspend{};
    };

public:
    static _generator<T, Alloc>::_type get_return_object_on_allocation_failure() 
    { 
        return { handle_type{} }; 
    }

    _generator<T, Alloc>::_type get_return_object() 
    { 
        return { handle_type::from_promise(*this) }; 
    }

    void return_void() noexcept 
    { 
        ::std::unique_lock lk{ m_mutex }; 
        m_finalized = true; 
        if (m_waitting)
        {
            wake_up(::std::move(m_waitting));
        }
    }

    void unhandled_exception() const { throw; }

    /*! \brief Function which stores the yield value.
     *  After store the yield value, this will makes the generator coroutine suspend and back to the caller function.
     */
    template<typename TT>
    auto yield_value(TT&& val)
    {
        ::std::unique_lock lk{ m_mutex };
        new (this->value_storage()) T(::std::forward<TT>(val));
        m_has_val = true;
        if (m_waitting)
        {
            wake_up(::std::move(m_waitting));
            m_waitting = {};
        }
        return ::std::suspend_always{};
    }

    /*! \retval true Current yield value was not been moved.
     *  \retval false Current yield value was moved out.
     */
    bool has_value() const noexcept
    {
        ::std::unique_lock lk{ m_mutex };
        return this->m_has_val;
    }

    bool has_value_impl() const noexcept
    {
        return !m_finalized && m_has_val;
    }

    /*! \brief Take the ownership of the current yield value.
     *  \return The current yield value object.
     */
    T value()
    {
        ::std::unique_lock lk{ m_mutex };
        return this->value_impl();
    }

    T value_impl()
    {
        T result = ::std::move(*this->value_storage());
        this->value_storage()->~T();
        m_has_val = false;
        return result;
    }

    ::std::optional<T> value_opt()
    {
        ::std::unique_lock lk{ m_mutex };
        return this->value_opt_impl();
    }

    ::std::optional<T> value_opt_impl()
    {
        ::std::optional<T> result{};
        if (this->has_value_impl())
        {
            result.emplace(value_impl());
        }
        return result;
    }

    bool finalized() const noexcept { ::std::unique_lock lk{ m_mutex }; return this->finalized_impl(); }
    bool finalized_impl() const noexcept { return m_finalized; }

    /*! \return Reference of the storage which holds the yield value and its memory buffer. */
    auto* value_storage() noexcept { return reinterpret_cast<T*>(&m_value_storage[0]); }
    const auto* value_storage() const noexcept { return reinterpret_cast<const T*>(&m_value_storage[0]); }

    /*! \brief destruct the current yield value and deallocate the memory. */
    void clear() noexcept { m_value_storage.reset(); }
};

/*! \brief The generator type
 *  \tparam T the yield value type.
 *  \tparam Alloc the allocator type.
 *
 *  This type will always holds this ownership of the coroutine handler.
 */
template<typename T, typename Alloc>
class _generator<T, Alloc>::_type
{
    static_assert(!::std::is_reference_v<T>, "The `result_type` of generator could not be a reference!");
public:
    friend class generator_promise_type<T, Alloc>;
    using promise_type = generator_promise_type<T, Alloc>;
    using result_type = T;
    using allocator = Alloc;

    /*! \brief Resume the execution of the generator, and update the yield value.
     *  \retval true Move successfully, you could take the newly yield value.
     *  \retval false Move failure, usually caused by there're actuall no more yield value.
     */
    bool move_next()
    {
        if (m_coro == nullptr)
            return false;
        m_coro.promise().clear();
        return (m_coro.resume(), !m_coro.done());
    }

    /*! \retval true There's a yield value you could retrive.
     *  \retval false There's no any yield value you could retrived.
     */
    bool has_value() const noexcept { return m_coro ? m_coro.promise().has_value() : false; }

    /*! \return the non-constant reference type of the yield value. 
     *  You could call this with `std::move`
     */
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

    /*! \return The reference of the storage which holds the memory buffer and the yield value.
     */
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

    ~_type() noexcept
    {
        if (m_need_destroy_in_dtor) m_coro.destroy();
    }

    _type& operator = (_type&& other) noexcept
    {
        this->destroy_current_coro();

        m_coro = ::std::exchange(other.m_coro, nullptr);
        m_need_destroy_in_dtor = ::std::exchange(other.m_need_destroy_in_dtor, false);

        return *this;
    }

private:
    _type(::std::coroutine_handle<promise_type> h) 
        : m_coro{ h } 
    {
    }

    ::std::coroutine_handle<promise_type> m_coro;
    bool m_need_destroy_in_dtor{ true };

public:
    using iterator = detial::sync_generator_iterator<_generator<T, Alloc>::_type>;
    iterator begin() noexcept { return { *this }; }
    constexpr detial::sync_generator_iterator_sentinel end() const noexcept { return {}; };

    [[nodiscard]] typename promise_type::get_yielded_aw next_value_async() & noexcept
    {
        return { m_coro };
    }

    template<typename PushBackAble>
    task<PushBackAble> to() &
    {
        PushBackAble result;
        ::std::optional<result_type> cur;
        for (;;) 
        {
            cur = co_await next_value_async();
            if (cur) result.push_back(::std::move(*cur));       
            else break;
        }

        co_return result;
    }

    template<template<typename...> class PushBackAble>
    task<PushBackAble<result_type>> to() &
    {
        return to<PushBackAble<result_type>>();
    }

    task<> to(auto iter) &
    {
        ::std::optional<result_type> cur;
        for (;;) 
        {
            cur = co_await next_value_async();
            if (cur) (*iter++) = ::std::move(*cur);
            else break;
        }
    }
};

template<typename T, typename Alloc = ::std::allocator<T>>
using generator = typename _generator<T, Alloc>::_type;

template<generator_concept Generator, typename Comp = ::std::less<typename Generator::result_type>>
Generator
merge(Generator lhs, Generator rhs, Comp comp = {})
{
    using T = Generator::result_type;
    ::std::optional<T> lhs_cur = co_await lhs.next_value_async();
    ::std::optional<T> rhs_cur = co_await rhs.next_value_async();
    for (;;)
    {
        if (lhs_cur && rhs_cur)
        {
            if (comp(*lhs_cur, *rhs_cur))
            {
                co_yield ::std::move(*lhs_cur);
                lhs_cur = co_await lhs.next_value_async();
            }
            else
            {
                co_yield ::std::move(*rhs_cur);
                rhs_cur = co_await rhs.next_value_async();
            }

        }
        else if (lhs_cur)
        {
            co_yield ::std::move(*lhs_cur);
            lhs_cur = co_await lhs.next_value_async();
        }
        else if (rhs_cur)
        {
            co_yield ::std::move(*rhs_cur);
            rhs_cur = co_await rhs.next_value_async();
        }
        else break;
    }
}

KOIOS_NAMESPACE_END

#endif
