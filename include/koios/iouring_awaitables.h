#ifndef KOIOS_IOURING_AWAITABLES_H
#define KOIOS_IOURING_AWAITABLES_H

#include "koios/iouring_accept_aw.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_read_aw.h"
#include "koios/iouring_recv_aw.h"
#include "koios/iouring_recvmsg_aw.h"
#include "koios/iouring_send_aw.h"
#include "koios/iouring_sendmsg_aw.h"
#include "koios/iouring_socket_aw.h"
#include "koios/iouring_sync_file_range_aw.h"
#include "koios/iouring_unlink_aw.h"
#include "koios/iouring_write_aw.h"
#include "koios/iouring_connect_aw.h"

#include "koios/task.h"
#include "toolpex/ipaddress.h"

namespace koios::uring
{
    ::koios::task<toolpex::unique_posix_fd> 
    bind_get_sock(toolpex::ip_address::uptr addr, in_port_t port, 
                  bool reuse_port = true, bool reuse_addr = true,
                  unsigned int flags = 0);
}

#endif
