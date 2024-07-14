// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_IOURING_AWAITABLES_H
#define KOIOS_IOURING_AWAITABLES_H

#include "koios/exceptions.h"
#include "koios/iouring_op_functions.h"

#include "koios/task.h"
#include "toolpex/ipaddress.h"
#include <cstddef>

namespace koios::uring
{
    /*! \brief  Awaitable: Connect the target machine then return the socket fd.
     *  \param  addr The ip address of target machine.
     *  \param  port The port.
     *  \param  socket_flags other flags of cqe.
     *  \return if there're no any error, the return value is the fd represents the connection.
     */
    task<toolpex::unique_posix_fd> 
    connect_get_sock(toolpex::ip_address::ptr addr, 
                     ::in_port_t port, 
                     unsigned int socket_flags = 0);

    /*! \brief  Awaitable: Bind a name with this process.
     *  \param  addr The ip address.
     *  \param  port The port.
     *  \param  socket_flags    other flags of cqe.
     *  \param  reuse_port      true if you need reuse port.
     *  \param  reuse_addr      true if you need reuse addr.
     *  \return if there're no any error, the return value is the fd represent the current socket.
     */
    task<toolpex::unique_posix_fd> 
    bind_get_sock_tcp(toolpex::ip_address::ptr addr, in_port_t port, 
                      bool reuse_port = true, bool reuse_addr = true,
                      unsigned int flags = 0);

    /*! \brief  Awaitable: Append all the bytes the buffer contained to the determined facility the fd represented.
     *  \param  fd The fd of the appendable facility.
     *  \param  buffer The bytes buffer.
     */
    task<>
    append_all(const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer);

    template<typename CharT>
    task<>
    inline append_all(const toolpex::unique_posix_fd& fd, 
                      ::std::basic_string_view<CharT, ::std::char_traits<CharT>> buffer)
    {
        return append_all(fd, ::std::as_bytes(
            ::std::span{ buffer.begin(), buffer.end() }
        ));
    }

    /*! \brief  Awaitable: Append all the bytes the buffer contained to the determined facility the fd represented.
     *  \param  fd The fd of the appendable facility.
     *  \param  buffer The bytes buffer.
     */
    task<>
    append_all(const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer, 
              ::std::error_code& ec_out) noexcept;

    template<typename CharT>
    inline task<>
    append_all(const toolpex::unique_posix_fd& fd, 
               ::std::basic_string_view<CharT, ::std::char_traits<CharT>> buffer,
               ::std::error_code& ec_out) noexcept
    {
        return append_all(fd, ::std::as_bytes(
            ::std::span{ buffer.begin(), buffer.end() }
        ), ec_out);
    }

    task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags,
                     ::std::error_code& ec, 
                     ::std::chrono::system_clock::time_point timeout 
                         = ::std::chrono::system_clock::time_point::max()) noexcept;

    task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags, 
                     ::std::error_code& ec, 
                     ::std::chrono::milliseconds dura) noexcept;

    task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags = 0, 
                     ::std::chrono::system_clock::time_point timeout 
                         = ::std::chrono::system_clock::time_point::max());

    task<size_t>
    recv_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int flags, 
                     ::std::chrono::milliseconds dura);

    task<size_t>
    read_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int offset,
                     ::std::error_code& ec, 
                     ::std::chrono::system_clock::time_point timeout 
                         = ::std::chrono::system_clock::time_point::max()) noexcept;

    task<size_t>
    read_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int offset, 
                     ::std::error_code& ec, 
                     ::std::chrono::milliseconds dura) noexcept;

    task<size_t>
    read_fill_buffer(const toolpex::unique_posix_fd& fd, 
                     ::std::span<::std::byte> buffer, 
                     int offset = 0, 
                     ::std::chrono::system_clock::time_point timeout 
                         = ::std::chrono::system_clock::time_point::max());

}

#endif
