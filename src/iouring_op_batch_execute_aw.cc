#include "koios/iouring_op_batch_execute_aw.h"
#include "koios/iouring_op_batch.h"
#include "koios/runtime.h"

namespace koios::uring
{

void op_batch_execute_aw::
await_suspend(task_on_the_fly t)
{
    m_rep->set_user_data(t.address());
    get_task_scheduler().add_event<iouring_event_loop>(
        ::std::move(t), 
        *m_rep
    );
}

} // namespace koios::uring
