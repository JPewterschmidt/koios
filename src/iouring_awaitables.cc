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

#include "koios/iouring_awaitables.h"
#include "toolpex/errret_thrower.h"

namespace koios::uring
{
    using namespace toolpex;

    ::koios::task<unique_posix_fd> 
    bind_get_sock(ip_address::ptr addr, in_port_t port, 
                  bool reuse_port, bool reuse_addr,
                  unsigned int flags)
    {
        auto sockret = co_await uring::socket(addr->family(), SOCK_STREAM, 0);
        if (auto ec = sockret.error_code(); ec)
        {
            throw posix_exception{ ec };
        }
        auto sockfd = sockret.get_socket_fd();

        errret_thrower et{};

        const int true_value{ 1 };
        const ::socklen_t vallen{ sizeof(true_value) };
        if (reuse_port) et << ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &true_value, vallen);
        if (reuse_addr) et << ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &true_value, vallen);

        const auto [sockaddr, size] = addr->to_sockaddr(port);

        errno = 0;
        et << ::bind(
            sockfd, 
            reinterpret_cast<const ::sockaddr*>(&sockaddr), 
            size
        );

        co_return sockfd;
    }

    ::koios::task<::std::error_code>
    append_all(const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer) noexcept
    try 
    {
        ::std::error_code result;
        ::std::span<const ::std::byte> left = buffer;

        while (!left.empty())
        {
            auto ret = co_await uring::write(fd, left);
            if ((result = ret.error_code())) break;
            left = left.subspan(ret.nbytes_delivered());
        }

        co_return result;
    }
    catch (const koios::exception& e)
    {
        co_return e.error_code();
    }
    catch (...)
    {
        co_return { KOIOS_EXCEPTION_CATCHED, koios_category() };
    }
}
