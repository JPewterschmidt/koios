#ifndef KOIOS_RETURN_VALUE_OR_VOID_H
#define KOIOS_RETURN_VALUE_OR_VOID_H

#include <future>
#include <coroutine>

#include "macros.h"

KOIOS_NAMESPACE_BEG

template<typename T>
class return_value_or_void_base
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
        task_scheduler_concept auto& scheduler = get_task_scheduler();
        scheduler.enqueue(m_caller);
    }
};

template<typename T>
class return_value_or_void : public return_value_or_void_base<T>
{
public: 
    template<typename TT>
    void return_value(TT&& val)
    {
        return_value_or_void_base<T>::m_promise.set_value(::std::forward<TT>(val));
        return_value_or_void_base<T>::wake_caller();
    }
};

template<>
class return_value_or_void<void> : public return_value_or_void_base<void>
{
public:
    void return_void() { return_value_or_void_base<void>::wake_caller(); }
};

KOIOS_NAMESPACE_END

#endif
