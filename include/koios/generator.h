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

#include "toolpex/object_storage.h"

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
class shared_state
{
private:
    task_on_the_fly m_generator_coro;
    task_on_the_fly m_waiting_coro;
    toolpex::object_storage<T> m_yielded_val;
    bool m_finalized{};

public:
    bool has_value() const noexcept
    {
        return !m_finalized && m_yielded_val.has_value();
    }

    void set_finalized() noexcept { m_finalized = true; }
    bool finalized() const noexcept { return m_finalized; }

    template<typename TT>
    void set_value(TT&& val)
    {
        m_yielded_val.set_value(::std::forward<TT>(val));
    }

    /*! \brief The function that mark then consumer side was die.
     *
     *  \attention How ever use this function has to make sure that the generator is in suspended mode.
     *             This function will be called by generator's destructor when for those calls not exhausts a generator.
     */
    void cancel() noexcept { set_finalized(); }

    void set_generator_coro(task_on_the_fly f) noexcept
    {
        toolpex_assert(!has_generator_coro());
        m_generator_coro = ::std::move(f);
    }

    auto get_generator_coro() noexcept
    {
        toolpex_assert(has_generator_coro());
        return ::std::move(m_generator_coro);
    }

    void set_waiting_coro(task_on_the_fly f) noexcept
    {
        toolpex_assert(!m_waiting_coro);
        m_waiting_coro = ::std::move(f);
    }

    auto get_waiting_coro() noexcept
    {
        // If there're no one still waiting for the generator, then nothing to return. 
        // Thus no need to check whether there's waiting coro handler.
        return ::std::move(m_waiting_coro);
    }

    bool has_waiting_coro() const noexcept { return !!m_waiting_coro; }
    bool has_generator_coro() const noexcept { return !!m_generator_coro; }

    auto& yielded_value_slot() noexcept { return m_yielded_val; }
    const auto& yielded_value_slot() const noexcept { return m_yielded_val; }
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
    : public promise_base<lazy_aw, destroy_aw>
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
   
    ~generator_promise_type() noexcept = default;

private:
    generator_detials::shared_state_sptr<T> m_shared_state{ ::std::make_shared<generator_detials::shared_state<T>>() };

public:
    class get_yielded_aw
    {
    public:
        get_yielded_aw(generator_detials::shared_state_sptr<T> ss) noexcept
            : m_ss{ ss }
        {
        }

        bool await_ready() noexcept
        {
            return m_ss->finalized();
        }

        void await_suspend(task_on_the_fly t) noexcept
        {
            m_ss->set_waiting_coro(::std::move(t));
            wake_up(m_ss->get_generator_coro());
        }

        ::std::optional<T> await_resume() 
        {
            ::std::optional<T> result;
            if (!m_ss->finalized())
                result.emplace(m_ss->yielded_value_slot().get_value());

            return result;
        }

    private:
        generator_detials::shared_state_sptr<T> m_ss;
    };

public:
    _generator<T, Alloc>::_type get_return_object() 
    { 
        return { handle_type::from_promise(*this), m_shared_state }; 
    }

    void return_void() noexcept 
    { 
        m_shared_state->set_finalized();
        wake_up(m_shared_state->get_waiting_coro());
    }

    void unhandled_exception() const { throw; }

    /*! \brief Function which stores the yield value.
     *  After store the yield value, this will makes the generator coroutine suspend and back to the caller function.
     */
    template<typename TT>
    auto yield_value(TT&& val)
    {
        toolpex_assert(!!m_shared_state);

        if (!finalized())
            m_shared_state->yielded_value_slot().set_value(::std::forward<TT>(val));

        struct yield_generator_getter_aw 
        { 
            yield_generator_getter_aw(generator_promise_type* parent, bool finalized) noexcept
                : m_parent{ parent }, m_ss{ m_parent->m_shared_state }, m_finalized{ finalized }
            {
            }

            constexpr bool await_ready() const noexcept { return false; }
            void await_suspend(task_on_the_fly h) noexcept
            {
                m_ss->set_generator_coro(m_finalized ? task_on_the_fly{} : ::std::move(h));
                wake_up(m_ss->get_waiting_coro());
            }
            
            constexpr void await_resume() const noexcept {}
            
            generator_promise_type* m_parent;
            generator_detials::shared_state_sptr<T> m_ss;
            bool m_finalized{};
        };

        return yield_generator_getter_aw{ this, finalized() };
    }

    /*! \retval true Current yield value was not been moved.
     *  \retval false Current yield value was moved out.
     */
    bool has_value() const noexcept
    {
        return m_shared_state->has_value();
    }

    /*! \brief Take the ownership of the current yield value.
     *  \return The current yield value object.
     */
    T value()
    {
        generator_detials::shared_state<T> c = m_shared_state->get_value();
        T result = ::std::move(c.m_yielded_val);
        return result;
    }

    ::std::optional<T> value_opt()
    {
        ::std::optional<T> result{};
        if (this->has_value())
        {
            result.emplace(value());
        }
        return result;
    }

    bool finalized() const noexcept { return m_shared_state->finalized(); }
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
        return m_shared_state->yielded_value_slot().value_ref();
    }

    _type(const _type&) = delete;
    _type(_type&& other) noexcept = default;
    _type& operator = (_type&& other) noexcept = default;
    ~_type() noexcept 
    {
        if (!m_shared_state)
            return;
        m_shared_state->cancel();
    }

private:
    _type(task_on_the_fly h, generator_detials::shared_state_sptr<T> storage) noexcept
        : m_shared_state{ ::std::move(storage) }
    {
        m_shared_state->set_generator_coro(::std::move(h));
    }

    generator_detials::shared_state_sptr<T> m_shared_state;

public:
    [[nodiscard]] typename promise_type::get_yielded_aw next_value_async() & noexcept
    {
        return { m_shared_state };
    }

private:
    template<generator_concept Gen>
    static Gen unique_impl(Gen gen, auto equal = ::std::equal_to<result_type>{})
    {
        auto compared_opt = co_await gen.next_value_async();
        if (!compared_opt.has_value())
            co_return;

        do
        {
            auto cur = co_await gen.next_value_async();
            if (!cur.has_value())
            {
                co_yield ::std::move(compared_opt.value());
                compared_opt = {};
            }
            else 
            {
                if (equal(compared_opt.value(), cur.value()))
                    continue;
                co_yield ::std::move(::std::exchange(compared_opt, cur).value());
            }
        }
        while (compared_opt.has_value());
    }

    template<typename PushBackAble>
    static task<PushBackAble> rvalue_to_impl(_type gen)
    {
        PushBackAble result;
        ::std::optional<result_type> cur;
        for (;;) 
        {
            cur = co_await gen.next_value_async();
            if (cur) result.push_back(::std::move(*cur));       
            else break;
        }

        co_return result;
    }

    static task<> rvalue_to_iter_impl(_type gen, auto iter)
    {
        ::std::optional<result_type> cur;
        for (;;) 
        {
            cur = co_await gen.next_value_async();
            if (cur) (*iter++) = ::std::move(*cur);
            else break;
        }
    }

public:
    template<typename Equal = ::std::equal_to<result_type>>
    _type unique(Equal eq = {}) &&
    {
        return unique_impl(::std::move(*this), ::std::move(eq));
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

    template<typename PushBackAble>
    task<PushBackAble> to() &&
    {
        return rvalue_to_impl<PushBackAble>(::std::move(*this));
    }

    template<template<typename...> class PushBackAble>
    task<PushBackAble<result_type>> to() &
    {
        return to<PushBackAble<result_type>>();
    }

    template<template<typename...> class PushBackAble>
    task<PushBackAble<result_type>> to() &&
    {
        return rvalue_to_impl<PushBackAble<result_type>>(::std::move(*this));
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

    task<> to(auto iter) &&
    {
        return rvalue_to_iter_impl(::std::move(*this), ::std::move(iter));
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
