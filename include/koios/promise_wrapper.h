// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_PROMISE_WRAPPER_H
#define KOIOS_PROMISE_WRAPPER_H

#include <coroutine>
#include <future>
#include <memory>

#include "koios/macros.h"
#include "koios/task_on_the_fly.h"
#include "koios/future.h"

KOIOS_NAMESPACE_BEG

/*! \brief Type Erasure class of promise. */
template<typename T>
class promise_wrapper
{
public:
    template<typename P>
    promise_wrapper(P& p) noexcept
        : m_promise{ ::std::addressof(p) }, 
          m_set_caller_impl{ +[](void* p, task_on_the_fly h) { static_cast<P*>(p)->set_caller(::std::move(h)); } }, 
          m_get_future_impl{ +[](void* p) { return static_cast<P*>(p)->get_future(); } }
    {
    }

    void set_caller(task_on_the_fly h) 
    {
        m_set_caller_impl(m_promise, ::std::move(h));
        m_caller_set = true;
    }
    
    koios::future<T> get_future()
    {
        return m_get_future_impl(m_promise);
    }

    bool caller_set() const noexcept { return m_caller_set; }

private:
    void* const m_promise;
    void (* const m_set_caller_impl)(void*, task_on_the_fly);
    koios::future<T> (* const m_get_future_impl)(void*);
    bool m_caller_set{};
};

KOIOS_NAMESPACE_END

#endif
