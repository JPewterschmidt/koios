// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/iouring_single_op_aw.h"

namespace koios::uring
{

read_write_aw::read_write_aw()      noexcept : op_batch_execute_aw{ batch().rep() } { }
socket_aw::socket_aw()              noexcept : op_batch_execute_aw{ batch().rep() } { }
accept_aw::accept_aw()              noexcept : op_batch_execute_aw{ batch().rep() } { }
connect_aw::connect_aw()            noexcept : op_batch_execute_aw{ batch().rep() } { }
cancel_aw::cancel_aw()              noexcept : op_batch_execute_aw{ batch().rep() } { }
fsync_aw::fsync_aw()                noexcept : op_batch_execute_aw{ batch().rep() } { }
normal_aw::normal_aw()              noexcept : op_batch_execute_aw{ batch().rep() } { }
nop_aw::nop_aw()                    noexcept : op_batch_execute_aw{ batch().rep() } { }
posix_result_aw::posix_result_aw()  noexcept : op_batch_execute_aw{ batch().rep() } { }

} // namespace koios::uring
