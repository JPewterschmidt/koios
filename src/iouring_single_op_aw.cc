/* koios, a c++ async runtime library.
 * copyright (c) 2024  jeremy pewterschmidt
 * 
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 * 
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 * 
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 */

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
