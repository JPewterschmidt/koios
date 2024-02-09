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

#include "koios/iouring_recv_aw.h"
#include <system_error>
#include <liburing.h>

namespace koios::uring { 

recv::recv(const toolpex::unique_posix_fd& fd, 
           ::std::span<::std::byte> buffer, 
           int flags)
{
    ::io_uring_prep_recv(
        sqe_ptr(), fd, 
        buffer.data(), static_cast<size_t>(buffer.size_bytes()), 
        //flags | IORING_RECVSEND_POLL_FIRST
        flags
    );
}

recv::recv(const toolpex::unique_posix_fd& fd, 
           ::std::span<unsigned char> buffer, 
           int flags)
    : recv(fd, as_writable_bytes(buffer), flags)
{
}

recv::recv(const toolpex::unique_posix_fd& fd, 
           ::std::span<char> buffer, 
           int flags)
    : recv(fd, as_writable_bytes(buffer), flags)
{
}

recv::recv(::std::chrono::milliseconds timeout, 
           const toolpex::unique_posix_fd& fd, 
           ::std::span<unsigned char> buffer, 
           int flags)
    : recv(fd, buffer, flags)
{
    set_timeout(timeout);
}

recv::recv(::std::chrono::milliseconds timeout, 
           const toolpex::unique_posix_fd& fd, 
           ::std::span<char> buffer, 
           int flags)
    : recv(fd, buffer, flags)
{
    set_timeout(timeout);
}

recv::recv(::std::chrono::milliseconds timeout,
           const toolpex::unique_posix_fd& fd, 
           ::std::span<::std::byte> buffer, 
           int flags)
    : recv(fd, buffer, flags)
{
    set_timeout(timeout);
}

} // namespace koios::uring
