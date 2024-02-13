#include "koios/iouring_single_op_aw.h"

namespace koios::uring
{

read_write_aw::read_write_aw()  noexcept : op_batch_execute_aw{ batch().rep() } { }
socket_aw::socket_aw()          noexcept : op_batch_execute_aw{ batch().rep() } { }
accept_aw::accept_aw()          noexcept : op_batch_execute_aw{ batch().rep() } { }
connect_aw::connect_aw()        noexcept : op_batch_execute_aw{ batch().rep() } { }
cancel_aw::cancel_aw()          noexcept : op_batch_execute_aw{ batch().rep() } { }
fsync_aw::fsync_aw()            noexcept : op_batch_execute_aw{ batch().rep() } { }

} // namespace koios::uring
