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

    task<toolpex::unique_posix_fd> 
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

    ::koios::task<>
    append_all(const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer)
    {
        ::std::error_code ec;
        co_await append_all(fd, buffer, ec);
        if (ec) [[unlikely]]
        {
            throw uring_exception(ec);
        }
    }

    ::koios::task<>
    append_all(const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer, 
              ::std::error_code& result) noexcept
    {
        ::std::span<const ::std::byte> left = buffer;

        while (!left.empty())
        {
            auto ret = co_await uring::write(fd, left);
            if ((result = ret.error_code())) break;
            left = left.subspan(ret.nbytes_delivered());
        }
    }

    static task<::std::span<::std::byte>> 
    after_read_or_recv(auto const& op_ret,
                       ::std::error_code& ec_out, 
                       size_t& wrote_nbytes, 
                       ::std::span<::std::byte> buffer)
    {
        switch (op_ret.error_code().value())
        {
            case 0:         break;
            default:        ec_out = op_ret.error_code(); [[fallthrough]];
            case ECANCELED: co_return {};
        }
        wrote_nbytes += op_ret.nbytes_delivered();
        co_return buffer.subspan(::std::min(op_ret.nbytes_delivered(), buffer.size()));
    }

    ::koios::task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags,
                     ::std::error_code& ec, 
                     ::std::chrono::system_clock::time_point timeout) noexcept
    try
    {
        size_t wrote_nbytes{};
        while (!ec && !buffer.empty() && ::std::chrono::system_clock::now() < timeout)
        {
            auto op_ret = co_await uring::recv(fd, buffer, flags, timeout);
            buffer = co_await after_read_or_recv(op_ret, ec, wrote_nbytes, buffer);
        }
        co_return wrote_nbytes;
    }
    catch (const koios::exception& e)
    {
        ec = e.error_code();
        co_return 0;
    }
    catch (...)
    {
        ec = ::std::error_code{ KOIOS_EXCEPTION_CATCHED, koios_category() };
        co_return 0;
    }

    ::koios::task<size_t>
    read_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int offset,
                     ::std::error_code& ec, 
                     ::std::chrono::system_clock::time_point timeout) noexcept
    try
    {
        size_t wrote_nbytes{};
        while (!ec && !buffer.empty() && ::std::chrono::system_clock::now() < timeout)
        {
            auto op_ret = co_await uring::read(fd, buffer, offset + wrote_nbytes, timeout);
            buffer = co_await after_read_or_recv(op_ret, ec, wrote_nbytes, buffer);
        }
        co_return wrote_nbytes;
    }
    catch (const koios::exception& e)
    {
        ec = e.error_code();
        co_return 0;
    }
    catch (...)
    {
        ec = ::std::error_code{ KOIOS_EXCEPTION_CATCHED, koios_category() };
        co_return 0;
    }

} // namespace koios::uring
