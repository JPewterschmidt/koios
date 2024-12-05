// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

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
        // Has to enable NONBLOCK since ther might be some async function relys on syscall outside iouring.
        auto sock_ret = co_await uring::socket(addr->family(), SOCK_STREAM | SOCK_NONBLOCK, 0, flags);
        if (auto ec = sock_ret.error_code(); ec)
            throw koios::uring_exception{ ec };
        auto sock = sock_ret.get_socket_fd();
        auto conn_ret = co_await uring::connect(sock, ::std::move(addr), port);
        if (auto ec = conn_ret.error_code(); ec)
            throw koios::uring_exception{ ec };

        co_return sock;
    }

    ::koios::task<unique_posix_fd> 
    bind_get_sock_tcp(ip_address::ptr addr, in_port_t port, 
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

    static ::std::span<::std::byte>
    after_read_or_recv(auto const& op_ret,
                       ::std::error_code& ec_out, 
                       size_t& wrote_nbytes, 
                       ::std::span<::std::byte> buffer)
    {
        switch (op_ret.error_code().value())
        {
            case 0:         break;
            default:        ec_out = op_ret.error_code(); [[fallthrough]];
            case ETIME:     return {};
        }
        wrote_nbytes += op_ret.nbytes_delivered();
        return buffer.subspan(::std::min(op_ret.nbytes_delivered(), buffer.size()));
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
            buffer = after_read_or_recv(op_ret, ec, wrote_nbytes, buffer);
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
            buffer = after_read_or_recv(op_ret, ec, wrote_nbytes, buffer);
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

    task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags, 
                     ::std::error_code& ec, 
                     ::std::chrono::milliseconds dura) noexcept
    {
        return recv_fill_buffer(fd, buffer, flags, ec,
                                dura + ::std::chrono::system_clock::now()
                               );
    }

    task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags, 
                     ::std::chrono::system_clock::time_point timeout)
    {
        ::std::error_code ec{};
        auto result = co_await recv_fill_buffer(fd, buffer, flags, ec, timeout);
        if (ec && ec != std_canceled_ec()) throw uring_exception(ec);
        co_return result;
    }

    task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags, 
                     ::std::chrono::milliseconds dura)
    {
        ::std::error_code ec{};
        auto result = co_await recv_fill_buffer(fd, buffer, flags, ec, dura);
        if (ec && ec != std_canceled_ec()) throw uring_exception(ec);
        co_return result;
    }

    task<size_t>
    read_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int offset, 
                     ::std::error_code& ec, 
                     ::std::chrono::milliseconds dura) noexcept
    {
        return read_fill_buffer(fd, buffer, offset, ec,
                                dura + ::std::chrono::system_clock::now()
                               );
    }

    task<size_t>
    read_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int offset, 
                     ::std::chrono::system_clock::time_point timeout)
    {
        ::std::error_code ec{};
        auto result = co_await read_fill_buffer(fd, buffer, offset, ec, timeout);
        if (ec && ec != std_canceled_ec()) throw uring_exception(ec);
        co_return result;
    }

} // namespace koios::uring
