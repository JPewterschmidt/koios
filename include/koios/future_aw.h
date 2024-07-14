// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include <memory>

#include "toolpex/assert.h"

#include "koios/task_on_the_fly.h"

namespace koios::fp_detials
{

class future_aw_detial
{
public:
    void set_waiting(task_on_the_fly t)
    {
        m_waiting = ::std::move(t);
    }

    void awake();

private:
    task_on_the_fly m_waiting;
};

template<typename FutureBase>
class future_aw
{
public:
    using value_type = FutureBase::value_type;

public:
    future_aw(FutureBase& fb) noexcept 
        : m_fb{ fb }, 
          m_fut_aw_detial{ ::std::make_unique<future_aw_detial>() }
    {
        toolpex_assert(m_fb.valid());
    }
    
    bool await_ready() const noexcept
    {
        return m_fb.ready();
    }

    void await_suspend(task_on_the_fly t) noexcept
    {
        m_fut_aw_detial->set_waiting(::std::move(t));
    }

    value_type await_resume()
    {
        return m_fb.get_nonblk();
    }

private:
    template<typename> friend class future_base;

    auto* get_aw_detial_ptr()
    {
        return m_fut_aw_detial.get();
    }

private:
    FutureBase& m_fb;
    ::std::unique_ptr<future_aw_detial> m_fut_aw_detial;
};

} // namespace koios::fp_detials

#endif
