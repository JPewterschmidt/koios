// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_TASK_ON_THE_FLY_H
#define KOIOS_TASK_ON_THE_FLY_H

#include <coroutine>
#include <utility>

#include "macros.h"

KOIOS_NAMESPACE_BEG

/*! \brief One of the `std::coroutine_handle` ownership phase manager.
 *
 *  When a task was scheduled in `task_scheduler`. 
 *  The ownership of the related `::std::coroutine_handle` will be taken by related `task_scheduler`, 
 *  be hold by this type of object immediately.
 *  When `quick_stop()` of `thread_pool` was called, there might be some task remain in the queue without executing.
 *  Meanwhile this type of objects are still hold the ownership of the `::std::coroutine_handle`.
 *  Then the destructor will ensure to call `destroy()`.
 *
 *  \see `destroy_aw`
 *  \see `task`
 */
class task_on_the_fly
{
public:
    constexpr task_on_the_fly() : m_h{}, m_holds_ownership{ false } {}

    task_on_the_fly(::std::coroutine_handle<> h) 
        : m_h{ h }
    {
    }

    template<typename T>
    task_on_the_fly(::std::coroutine_handle<T> h)
        : m_h{ h }
    {
    }

    task_on_the_fly(task_on_the_fly&& other) noexcept
        : m_h{ other.m_h }, m_holds_ownership{ other.exchange_ownership() }
    {
    }

    task_on_the_fly& operator=(task_on_the_fly&& other) noexcept
    {
        this->destroy();
        m_h = other.m_h;
        m_holds_ownership = other.exchange_ownership();

        return *this;
    }

    task_on_the_fly(const task_on_the_fly&) = delete;
    task_on_the_fly& operator=(const task_on_the_fly&) = delete;

    /*! \brief Execute and give up the ownership of the related `std::coroutine_handle`.
     *
     *  This member function will firstly release the ownership of the handler, 
     *  secondly call the `std::coroutine_handle::resume()` of the handler, 
     */
    void operator()()
    { 
        this->give_up_ownership();
        m_h.resume(); 
    }

    operator bool() const noexcept { return this->holds_ownership() && m_h; }
    bool valid() const noexcept { return this->operator bool(); }

    bool done() const noexcept
    {
        if (this->holds_ownership()) [[likely]] return m_h.done();
        return true;
    }

    /*! If the `operator()()` not called, in other words, 
     *  this `task_on_the_fly` still holds the ownership of the handler, 
     *  the destructor will call `::std::coroutine_handle::destroy()`.
     */
    ~task_on_the_fly() noexcept { this->destroy(); }

    void* address() const noexcept
    {
        if (!holds_ownership()) [[unlikely]] return nullptr;
        return m_h.address();
    }

    void give_up_ownership() noexcept { this->exchange_ownership(); }

private:
    void destroy() noexcept
    {
        if (!this->holds_ownership()) return;

        this->give_up_ownership();
        m_h.destroy();
    }

    bool exchange_ownership() noexcept { return ::std::exchange(m_holds_ownership, false); }
    bool holds_ownership() const noexcept { return m_holds_ownership; }

private:
    ::std::coroutine_handle<> m_h;

    // instead of using this member directly, 
    // use `exchange_ownership()`, `give_up_ownership()`, `holds_ownership()` instead.
    bool m_holds_ownership{ true }; 
};

KOIOS_NAMESPACE_END

#endif
