/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
#include "koios/iouring_fsync_aw.h"
#include "koios/iouring_unlink_aw.h"
#include "koios/iouring_write_aw.h"
#include "koios/iouring_connect_aw.h"

#include "koios/task.h"
#include "toolpex/ipaddress.h"
#include <cstddef>

namespace koios::uring
{
    ::koios::task<toolpex::unique_posix_fd> 
    bind_get_sock(toolpex::ip_address::ptr addr, in_port_t port, 
                  bool reuse_port = true, bool reuse_addr = true,
                  unsigned int flags = 0);

    ::koios::task<::std::error_code>
    append_all(const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer) noexcept;
}

#endif
