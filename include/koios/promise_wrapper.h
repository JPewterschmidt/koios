#ifndef KOIOS_PROMISE_WRAPPER_H
#define KOIOS_PROMISE_WRAPPER_H

#include "macros.h"
#include <coroutine>
#include <future>
#include <memory>

KOIOS_NAMESPACE_BEG

template<typename T>
class promise_wrapper
{
public:
    template<typename P>
    promise_wrapper(P& p) noexcept
        : m_promise{ &p }, 
          m_set_caller_impl{ [](void* p, ::std::coroutine_handle<> h) { static_cast<P*>(p)->set_caller(h); } }, 
          m_get_future_impl{ [](void* p) { return static_cast<P*>(p)->get_future(); } }
    {
    }

    void set_caller(::std::coroutine_handle<> h) 
    {
        m_set_caller_impl(m_promise, h);
    }
    
    ::std::future<T> get_future()
    {
        return m_get_future_impl(m_promise);
    }

private:
    void* const m_promise;
    void (* const m_set_caller_impl)(void*, ::std::coroutine_handle<>);
    ::std::future<T> (* const m_get_future_impl)(void*);
};

KOIOS_NAMESPACE_END

#endif
