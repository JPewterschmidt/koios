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

#include "koios/iouring_sendmsg_aw.h"

#include <system_error>
#include <liburing.h>

namespace koios::uring { 

sendmsg::sendmsg(const toolpex::unique_posix_fd& fd, 
                 const ::msghdr* msg, 
                 int flags)
{
    ::io_uring_prep_sendmsg(sqe_ptr(), fd, msg, flags);
}

sendmsg::sendmsg(::std::chrono::milliseconds timeout, 
                 const toolpex::unique_posix_fd& fd, 
                 const ::msghdr* msg, 
                 int flags)
    : sendmsg(fd, msg, flags)
{
    set_timeout(timeout);
}

sendmsg::sendmsg(::std::chrono::system_clock::time_point timeout, 
                 const toolpex::unique_posix_fd& fd, 
                 const ::msghdr* msg, 
                 int flags)
    : sendmsg(fd, msg, flags)
{
    set_timeout(timeout);
}

} // namespace koios::uring
