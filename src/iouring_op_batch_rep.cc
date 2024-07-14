// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/iouring_op_batch_rep.h"
#include <iterator>

namespace koios::uring
{

::io_uring_sqe* op_batch_rep::get_sqe()
{
    if (!this->was_timeout_set())
        return &m_sqes.emplace_back();
    return &(*m_sqes.emplace(prev(m_sqes.end())));
}

void op_batch_rep::set_user_data(void* userdata)
{
    for (auto& sqe : m_sqes)
        ::io_uring_sqe_set_data(&sqe, userdata);
}

void op_batch_rep::set_user_data(uintptr_t userdata)
{
    for (auto& sqe : m_sqes)
        ::io_uring_sqe_set_data64(&sqe, userdata);
}

}
