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
#include "koios/generator_concepts.h"
#include "koios/task_on_the_fly.h"
#include "koios/waiting_handle.h"
#include "koios/task.h"

KOIOS_NAMESPACE_BEG

namespace generator_detials
{

template<typename T>
struct shared_state
{
    task_on_the_fly m_generator_coro;
    task_on_the_fly m_waitting_coro;
    toolpex::object_storage m_yielded_val;
    bool m_finalzed{};

    bool has_value() const noexcept
    {
        return !m_finalized && m_yielded_val.has_value();
    }

    bool finalized() const noexcept
    {
        return m_finalized;
    }

    template<typename TT>
    void set_value(TT&& val)
    {
        m_yielded_val.set_value(::std::forward<TT>(val));
    }
};

template<typename T>
using shared_state_sptr = ::std::shared_ptr<shared_state<T>>;

} // namespace generator_detials

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
    task_on_the_fly m_waitting{};
    bool m_finalized{};
    generator_detials::shared_state_sptr m_shared_state{ ::std::make_shared<shared_state<T>>() };

public:
    class get_yielded_aw
    {
    public:
        get_yielded_aw(generator_detials::shared_state* ss) noexcept
            : m_ss{ ss }
        {
        }

        bool await_ready() noexcept
        {
            return m_ss->finalized();
        }

        void await_suspend(task_on_the_fly t) noexcept
        {
            m_ss->m_waitting_coro = ::std::move(t);
            toolpex_assert(!!m_ss->m_generator_coro);
            wake_up(::std::move(m_ss->m_generator_coro);
        }

        ::std::optional<T> await_resume() 
        {
            toolpex_assert(!m_debug_after_resume);
            ::std::optional<T> result;
            if (!m_ss->m_finalized)
                result->emplace(m_ss->m_yielded_val.get_value());

            return result;
        }

    private:
        generator_detials::shared_state* m_ss;
    };

public:
    static _generator<T, Alloc>::_type get_return_object_on_allocation_failure() 
    { 
        return { handle_type{} }; 
    }

    _generator<T, Alloc>::_type get_return_object() 
    { 
        return { handle_type::from_promise(*this), m_shared_state }; 
    }

    void return_void() noexcept 
    { 
        m_shared_state->m_finalized = true;
        wake_up(m_shared_state->m_waitting_coro);
    }

    void unhandled_exception() const { throw; }

    /*! \brief Function which stores the yield value.
     *  After store the yield value, this will makes the generator coroutine suspend and back to the caller function.
     */
    template<typename TT>
    auto yield_value(TT&& val)
    {
        toolpex_assert(!!m_shared_state);
        m_shared_state->m_yielded_val.set_value(::std::forward<TT>(val));

        struct yield_generator_getter_aw 
        { 
            yield_generator_getter_aw(generator_promise_type* parent) noexcept
                : m_parent{ parent }
            {
            }

            constexpr bool await_ready() const noexcept { return false; }
            void await_suspend(task_on_the_fly h) noexcept
            {
                auto& ss = *m_parent->m_shared_state;
                ss.m_generator_coro = ::std::move(h);
                wake_up(::std::move(ss.m_waitting_coro));
            }
            
            constexpr void await_resume() const noexcept {}
            
            generator_promise_type* m_parent;
            generator_detials::shared_state* m_ss;
        };

        return yield_generator_getter_aw{ this };
    }

    /*! \retval true Current yield value was not been moved.
     *  \retval false Current yield value was moved out.
     */
    bool has_value() const noexcept
    {
        return !m_finalized && m_has_val;
    }

    /*! \brief Take the ownership of the current yield value.
     *  \return The current yield value object.
     */
    T value()
    {
        generator_detials::shared_state c = m_shared_state->get_value();
        T result = ::std::move(c.m_yielded_val);
        return result;
    }

    ::std::optional<T> value_opt()
    {
        return this->value_opt_impl();
    }

    ::std::optional<T> value_opt_impl()
    {
        ::std::optional<T> result{};
        if (this->has_value())
        {
            result.emplace(value_impl());
        }
        return result;
    }

    bool finalized() const noexcept { return m_finalized; }

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

    /*! \retval true There's a yield value you could retrive.
     *  \retval false There's no any yield value you could retrived.
     */
    bool has_value() const noexcept 
    { 
        toolpex_assert(!!m_shared_state);
        return m_shared_state->has_value();
    }

    /*! \return the non-constant reference type of the yield value. 
     *  You could call this with `std::move`
     */
    decltype(auto) current_value()
    {
        if (!m_shared_state->has_value()) 
        {
            throw ::std::out_of_range{ 
                "There's no next val could be retrived, call `has_value()` first! "
                "Or, you may perform co_await call, it's not supported yet."
            };
        }
        return m_shared_state->m_yielded_val.value_ref();
    }

    _type(const _type&) = delete;
    _type(_type&& other) noexcept = default;
    _type& operator = (_type&& other) noexcept = default;

private:
    _type(task_on_the_fly h, ::std::shared_ptr<toolpex::object_storage<::std::pair<result_type, task_on_the_fly>>> storage) noexcept
        : m_shared_state{ ::std::move(storage) }
    {
        m_shared_state->m_generator_coro = ::std::move(h);
    }

    ::std::shared_ptr<toolpex::object_storage<generator_detials::shared_state<T>>> m_shared_state;

public:
    [[nodiscard]] typename promise_type::get_yielded_aw next_value_async() & noexcept
    {
        return { m_shared_state };
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
