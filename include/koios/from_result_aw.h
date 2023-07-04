#ifndef KOIOS_FROM_RESULT_AW_H
#define KOIOS_FROM_RESULT_AW_H

#include <coroutine>
#include <utility>

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

template<typename Ret>
class from_result_aw
{
public: 
    template<typename... Args>
    constexpr from_result_aw(Args&&... r)
        : m_obj{ ::std::forward<Args>(r)... }
    {
    }

    constexpr bool await_ready() const noexcept { return true; }
    constexpr void await_suspend(::std::coroutine_handle<>) const noexcept {}
    constexpr Ret await_resume() noexcept { return ::std::move(m_obj); }

private:
    Ret m_obj;
};

KOIOS_NAMESPACE_END

#endif
