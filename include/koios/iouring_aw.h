#ifndef KOIOS_IOURING_AW_H
#define KOIOS_IOURING_AW_H

#include "koios/macros.h"
#include "koios/iouring.h"
#include "koios/runtime.h"

KOIOS_NAMESPACE_BEG

class iouring_aw
{
public:
    iouring_aw(::io_uring_sqe sqe);
    constexpr bool await_ready() const noexcept { return false; }
    
    void await_suspend(task_on_the_fly h);

    ioret await_resume() 
    { 
        return *m_ret;
    }

private:
    ::std::shared_ptr<ioret> m_ret;
    ::io_uring_sqe m_sqe{};
};

KOIOS_NAMESPACE_END

#endif
