#include "koios/iouring_aw.h"
#include "koios/runtime.h"

KOIOS_NAMESPACE_BEG

namespace uring { 

iouring_aw::iouring_aw(::io_uring_sqe sqe) 
    : m_ret{ ::std::make_shared<ioret>() }, 
      m_sqe{ sqe }
{
}

void iouring_aw::
await_suspend(task_on_the_fly h) 
{
    koios::get_task_scheduler().add_event<iouring_event_loop>(
        ::std::move(h), m_ret, m_sqe
    );
}

} // namespace uring

KOIOS_NAMESPACE_END
