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

#include "koios/iouring_connect_aw.h"
#include "koios/iouring_socket_aw.h"

namespace koios::uring
{
    connect::connect(const toolpex::unique_posix_fd& fd, 
                     toolpex::ip_address::ptr addr, 
                     ::in_port_t port)
        : detials::iouring_aw_for_connect(io_uring_sqe{})
    {
        auto [sock, len] = addr->to_sockaddr(port);
        m_sockaddr = sock;

        ::io_uring_prep_connect(
            sqe_ptr(), fd, 
            reinterpret_cast<sockaddr*>(&m_sockaddr), len
        ); 
    }

    connect::connect(::std::chrono::milliseconds timeout, 
                     const toolpex::unique_posix_fd& fd, 
                     toolpex::ip_address::ptr addr, 
                     ::in_port_t port)
        : connect(fd, ::std::move(addr), port)
    {
        set_timeout(timeout);
    }

    ::koios::task<toolpex::unique_posix_fd> 
    connect_get_sock(toolpex::ip_address::ptr addr, 
                     ::in_port_t port, 
                     unsigned int flags)
    {
        auto sock_ret = co_await uring::socket(addr->family(), SOCK_STREAM, 0, flags);
        if (auto ec = sock_ret.error_code(); ec)
            throw koios::uring_exception{ ec };
        auto sock = sock_ret.get_socket_fd();
        auto conn_ret = co_await uring::connect(sock, ::std::move(addr), port);
        if (auto ec = conn_ret.error_code(); ec)
            throw koios::uring_exception{ ec };

        co_return sock;
    }
}
