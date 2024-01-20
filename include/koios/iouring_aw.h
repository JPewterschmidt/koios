#ifndef KOIOS_IOURING_AW_H
#define KOIOS_IOURING_AW_H

#include "koios/macros.h"
#include "koios/iouring_ioret.h"
#include "koios/task_on_the_fly.h"
#include <liburing.h>
#include <memory>
#include <cstdint>
#include <cstddef>
#include <span>

namespace koios::uring
{
    class iouring_aw
    {
    public:
        iouring_aw(::io_uring_sqe sqe);
        constexpr bool await_ready() const noexcept { return false; }
        
        void await_suspend(task_on_the_fly h);

        detials::ioret_for_any_base await_resume() 
        { 
            return *m_ret;
        }

    private:
        ::std::shared_ptr<ioret> m_ret;
        ::io_uring_sqe m_sqe{};
    };
}

#endif
