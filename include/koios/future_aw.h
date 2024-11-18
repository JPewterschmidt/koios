// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_FUTURE_AW_H
#define KOIOS_FUTURE_AW_H

#include <memory>

#include "toolpex/assert.h"

#include "koios/task_on_the_fly.h"

namespace koios
{

template<typename FutureBase>
class future_aw
{
public:
    using value_type = FutureBase::value_type;

public:
    future_aw(FutureBase& fb) noexcept 
        : m_future{ fb }
    {
        toolpex_assert(m_future.valid());
    }
    
    bool await_ready() noexcept
    {
        return m_future.ready();
    }

    void await_suspend(task_on_the_fly t) noexcept
    {
        m_future.set_waiting(::std::move(t));
    }

    value_type await_resume()
    {
        return m_future.get_nonblk();
    }

private:
    FutureBase& m_future;
};

} // namespace koios

#endif
