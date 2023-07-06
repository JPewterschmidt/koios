#ifndef KOIOS_FROM_RESULT_H
#define KOIOS_FROM_RESULT_H

#include <coroutine>
#include <utility>

#include "koios/macros.h"
#include "koios/task.h"

KOIOS_NAMESPACE_BEG

namespace from_result_detial
{
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
}

template<typename Ret>
sync_task<Ret> from_result(Ret r)
{
    co_return r;
}

KOIOS_NAMESPACE_END

#endif
