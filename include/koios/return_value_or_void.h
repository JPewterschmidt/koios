#ifndef KOIOS_RETURN_VALUE_OR_VOID_H
#define KOIOS_RETURN_VALUE_OR_VOID_H

#include <future>
#include <coroutine>

#include "koios/macros.h"
#include "koios/task_scheduler_selector.h"

KOIOS_NAMESPACE_BEG

template<typename T, typename Promise>
class return_value_or_void_base 
    : public virtual task_scheduler_selector
{
public:
    void set_caller(::std::coroutine_handle<> h) noexcept { m_caller = h; }
    auto get_future() { return m_promise.get_future(); }

protected:
    ::std::promise<T> m_promise;
    ::std::coroutine_handle<> m_caller{};

    void wake_caller()
    {
        if (!m_caller) return;
        scheduler().enqueue(m_caller);
    }
};

template<typename T, typename Promise>
class return_value_or_void 
    : public return_value_or_void_base<T, Promise>
{
public: 
    template<typename TT>
    void return_value(TT&& val)
    {
        return_value_or_void_base<T, Promise>::m_promise.set_value(::std::forward<TT>(val));
        return_value_or_void_base<T, Promise>::wake_caller();
    }
};

template<typename Promise>
class return_value_or_void<void, Promise> 
    : public return_value_or_void_base<void, Promise>
{
public:
    void return_void() 
    { 
        return_value_or_void_base<void, Promise>::wake_caller(); 
    }
};

KOIOS_NAMESPACE_END

#endif
