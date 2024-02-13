#include "koios/iouring_op_batch_rep.h"

namespace koios::uring
{

void op_batch_rep::set_user_data(void* userdata)
{
    for (auto& sqe : m_sqes)
        ::io_uring_sqe_set_data(&sqe, userdata);
}

void op_batch_rep::set_user_data(uint64_t userdata)
{
    for (auto& sqe : m_sqes)
        ::io_uring_sqe_set_data64(&sqe, userdata);
}

}
