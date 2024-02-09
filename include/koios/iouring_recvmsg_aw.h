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

#ifndef KOIOS_IOURING_RECVMSG_AW_H
#define KOIOS_IOURING_RECVMSG_AW_H

#include <system_error>
#include <cerrno>
#include <span>
#include <sys/types.h>
#include <sys/socket.h>

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_detials.h"

#include "toolpex/unique_posix_fd.h"

namespace koios::uring
{
    class recvmsg : public detials::iouring_aw_for_data_deliver
    {
    public:
        recvmsg(const toolpex::unique_posix_fd& fd, 
                ::msghdr* msg, 
                int flags = 0);

        recvmsg(::std::chrono::milliseconds timeout, 
                const toolpex::unique_posix_fd& fd, 
                ::msghdr* msg, 
                int flags = 0);

        recvmsg(::std::chrono::system_clock::time_point timeout, 
                const toolpex::unique_posix_fd& fd, 
                ::msghdr* msg, 
                int flags = 0);
    };
}

#endif
