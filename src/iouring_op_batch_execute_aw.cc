// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/iouring_op_batch_execute_aw.h"
#include "koios/iouring_op_batch.h"
#include "koios/runtime.h"
#include "toolpex/bit_mask.h"

namespace koios::uring
{

void op_batch_execute_aw::
await_suspend(task_on_the_fly t)
{
    m_rep->set_user_data(t.address());
    auto& last_sqe = m_rep->back();
    last_sqe.flags = toolpex::bit_mask{last_sqe.flags}
       .remove(static_cast<uint8_t>(IOSQE_IO_LINK));

    get_task_scheduler().add_event<iouring_event_loop>(
        ::std::move(t), 
        *m_rep
    );
}

} // namespace koios::uring
