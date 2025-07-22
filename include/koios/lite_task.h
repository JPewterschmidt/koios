// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2025, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_LITE_TASK_H
#define KOIOS_LITE_TASK_H

#include <utility>
#include <memory>
#include <source_location>
#include <mutex>
#include <concepts>
#include <string>
#include <string_view>
#include <span>
#include <cstddef>

#include "koios/macros.h"
#include "koios/promise_base.h"
#include "koios/return_value_or_void.h"
#include "koios/task_scheduler.h"
#include "koios/task_on_the_fly.h"
#include "koios/lite_future.h"
#include "koios/per_consumer_attr.h"
#include "koios/discardable_mark.h"

namespace koios
{

template<typename, typename, typename>
struct _lite_task
{
    struct [[nodiscard]] _type;
};

template<
    typename T, 
    typename Discardable, 
    typename InitialSuspendAw>
class _lite_task<T, Discardable, InitialSuspendAw>::_type 
{
public:
    using value_type = T;
    using future_type = koios::lite_future<value_type>;
    using initial_suspend_type = InitialSuspendAw;

    // This should be always a move only object, 
    // `return_value_or_void` was derived from `toolpex::move_only`
    // See blog https://devblogs.microsoft.com/oldnewthing/20210504-00/?p=105176
    // by Raymond Chen
    class promise_type 
        : public promise_base<InitialSuspendAw, destroy_aw>, 
          public return_value_or_void<T, koios::lite_promise<T>>
    {
    public:
        _lite_task<T, Discardable, initial_suspend_type>::_type 
        get_return_object() noexcept { return { *this }; }
        void unhandled_exception() { this->deal_exception(::std::current_exception()); }
    };

    friend class task_scheduler;

protected:
    _type(promise_type& p)
        : m_coro_handle{ ::std::coroutine_handle<promise_type>::from_promise(p) }, 
          m_future{ p.get_future() }
    {
        if constexpr (this->is_eager()) m_coro_handle.give_up_ownership();
    }

public:
    /*! Of course move constructor will move the ownership of the handler. */
    _type(_type&& other) noexcept
        : m_coro_handle{ ::std::move(other.m_coro_handle) }, 
          m_future{ ::std::move(other.m_future) }
    {
    }

    operator task_on_the_fly() noexcept { return this->get_handler_to_schedule(); }

    /*! \retval true The coroutine state was suspended at final state, 
     *          or the task has been executed by `run()` or other similar functions.
     *  \retval false The coroutine state was suspended at state which is NOT final.
     */
    bool done() const noexcept { return m_coro_handle.done(); }

    auto operator co_await()
    {
        if (!m_future.valid())
        {
            throw ::std::logic_error{ "task::operator co_await(): you have already called task::get_future()." };
        }
        get_task_scheduler().enqueue(this->get_handler_to_schedule());
        return m_future.get_async();
    }

    template<typename Dummy = void>
    void run()
    {
        static_assert(this->is_discardable(), "you can not run a non-discardable task without retrieving it's return value.");
        if (!m_future.valid())
        {
            throw ::std::logic_error{ "task::operator co_await(): you have already called task::get_future()." };
        }
        get_task_scheduler().enqueue(this->get_handler_to_schedule());
    }

    /*! \retval true This task is a discardable task. You could ignore the return value.
     *  \retval false This task is NOT a Discardable task. You have to take the ownership of the related future object.
     *
     *  And this is a static consteval function.
     */
    [[nodiscard]] static consteval bool is_discardable() { return ::std::same_as<Discardable, discardable>; }
    [[nodiscard]] static consteval bool is_eager() { return ::std::same_as<initial_suspend_type, eager_aw>; }

private:
    [[nodiscard]] static consteval bool is_return_void() { return ::std::same_as<void, value_type>; }
    [[nodiscard]] bool has_scheduled() const noexcept { return !m_coro_handle; }

    /*! \brief Take the ownership of the future object related to this task.
     *  \return the future object.
     *  \see `std::future`.
     *
     *  \warning You can only call this fuction only once BEFORE `get_future()` and `run()`.
     *           Or you will get a `std::logic_error`.
     */
    [[nodiscard]] future_type get_future()
    {
        if constexpr (!this->is_eager())
        {
            if (this->has_scheduled()) throw ::std::logic_error{ "You should call `get_future()` before `run()`" };
        }

        return ::std::move(m_future);
    }

    auto get_handler_to_schedule() noexcept { return ::std::exchange(m_coro_handle, {}); }

private:
    task_on_the_fly m_coro_handle;
    future_type m_future;
};

template<typename T = void, typename Discardable = discardable, typename InitialSuspendAw = eager_aw>
using async_lite_task = _lite_task<T, Discardable, InitialSuspendAw>::_type;

template<typename T = void, typename Discardable = non_discardable, typename InitialSuspendAw = eager_aw>
using nodiscard_lite_task = _lite_task<T, Discardable, InitialSuspendAw>::_type;

template<typename T = void, typename InitialSuspendAw = eager_aw>
using lite_task = async_lite_task<T, InitialSuspendAw>;

template<typename T = void, typename InitialSuspendAw = lazy_aw>
using lazy_lite_task = async_lite_task<T, InitialSuspendAw>;

using litetaskec = lite_task<::std::error_code>;
using lzlitetaskec = lazy_lite_task<::std::error_code>;

} // namespace koios

extern template class koios::_lite_task<void, koios::discardable, ::std::suspend_always>::_type;
extern template class koios::_lite_task<void, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<void, koios::non_discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<bool, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<int, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<size_t, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<::std::string, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<::std::string_view, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<::std::error_code, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<uint8_t, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<uint32_t, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<::std::byte*, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<const ::std::byte*, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<char*, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<const char*, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<void*, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<const void*, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<::std::span<::std::byte>, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<::std::span<const ::std::byte>, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<::std::span<char>, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<::std::span<const char>, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<::std::span<uint8_t>, koios::discardable, koios::eager_aw>::_type;
extern template class koios::_lite_task<::std::span<const uint8_t>, koios::discardable, koios::eager_aw>::_type;

#endif
