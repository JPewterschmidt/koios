// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

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
        constexpr from_result_aw(Ret r)
            : m_obj{ ::std::move(r) }
        {
        }

        constexpr bool await_ready() const noexcept { return true; }
        constexpr void await_suspend(::std::coroutine_handle<>) const noexcept {}
        constexpr Ret await_resume() noexcept { return ::std::move(m_obj); }

    private:
        Ret m_obj;
    };
}

template<typename T>
auto from_result(T&& r)
{
    return from_result_detial::from_result_aw(::std::forward<T>(r));
}

KOIOS_NAMESPACE_END

#endif
