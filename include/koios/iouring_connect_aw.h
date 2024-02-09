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

#ifndef KOIOS_IOURING_CONNECT_AW_H
#define KOIOS_IOURING_CONNECT_AW_H

#include "koios/macros.h"
#include "toolpex/ipaddress.h"
#include "koios/iouring_aw.h"
#include "toolpex/unique_posix_fd.h"
#include "koios/iouring_detials.h"
#include "koios/task.h"

namespace koios::uring
{
    class connect : public detials::iouring_aw_for_connect
    {
    public:
        connect(const toolpex::unique_posix_fd& fd, 
                toolpex::ip_address::ptr addr, 
                ::in_port_t port);

        connect(::std::chrono::milliseconds timeout, 
                const toolpex::unique_posix_fd& fd, 
                toolpex::ip_address::ptr addr, 
                ::in_port_t port);

    private:
        ::sockaddr_storage m_sockaddr{};
    };

    ::koios::task<toolpex::unique_posix_fd> 
    connect_get_sock(toolpex::ip_address::ptr addr, 
                     ::in_port_t port, 
                     unsigned int socket_flags = 0);
}
#endif
