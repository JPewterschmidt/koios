#ifndef KOIOS_IOURING_OP_FUNCTIONS_H
#define KOIOS_IOURING_OP_FUNCTIONS_H

#include "koios/iouring_single_op_aw.h"
#include "toolpex/unique_posix_fd.h"
#include "toolpex/concepts_and_traits.h"

namespace koios::uring
{

template<typename T, ::std::size_t Extent>
read_write_aw 
read(const toolpex::unique_posix_fd& fd, 
     ::std::span<T, Extent> buffer, 
     uint64_t offset = 0)
{
    read_write_aw result;
    result.batch().prep_read(fd, buffer, offset);
    return result;
}

template<typename T, ::std::size_t Extent>
read_write_aw 
read(toolpex::is_std_chrono_duration_or_time_point auto tmot,
     const toolpex::unique_posix_fd& fd, 
     ::std::span<T, Extent> buffer, 
     uint64_t offset = 0)
{
    read_write_aw result;
    result.batch().prep_read(fd, buffer, offset).timeout(tmot);
    return result;
}

template<typename T, ::std::size_t Extent>
read_write_aw 
write(const toolpex::unique_posix_fd& fd, 
      ::std::span<T, Extent> buffer, 
      uint64_t offset = 0)
{
    read_write_aw result{};
    result.batch().prep_write(fd, ::std::as_bytes(buffer), offset);
    return result;
}

template<typename T, ::std::size_t Extent>
read_write_aw 
write(toolpex::is_std_chrono_duration_or_time_point auto tmot, 
      const toolpex::unique_posix_fd& fd, 
      ::std::span<const unsigned char> buffer, 
      uint64_t offset = 0)
{
    read_write_aw result;
    result.batch()
        .prep_write(fd, ::std::as_bytes(buffer), offset)
        .timeout(tmot);
    return result;
}

template<typename T, ::std::size_t Extent>
recv_send_aw
send(const toolpex::unique_posix_fd& fd, 
     ::std::span<T, Extent> buffer, 
     int flags = 0)
{
    recv_send_aw result{};
    result.batch().prep_send(fd, buffer, flags);
    return result;
}

template<typename T, ::std::size_t Extent>
recv_send_aw
send(toolpex::is_std_chrono_duration_or_time_point auto tmot, 
     const toolpex::unique_posix_fd& fd, 
     ::std::span<T, Extent> buffer, 
     int flags = 0)
{
    recv_send_aw result{};
    result.batch().prep_send(fd, buffer, flags).timeout(tmot);
    return result;
}

template<typename T, ::std::size_t Extent>
recv_send_aw
recv(const toolpex::unique_posix_fd& fd, 
     ::std::span<T, Extent> buffer, 
     int flags = 0)
{
    recv_send_aw result{};
    result.batch().prep_recv(fd, buffer, flags);
    return result;
}

template<typename T, ::std::size_t Extent>
recv_send_aw
recv(toolpex::is_std_chrono_duration_or_time_point auto tmot, 
     const toolpex::unique_posix_fd& fd, 
     ::std::span<T, Extent> buffer, 
     int flags = 0)
{
    recv_send_aw result{};
    result.batch().prep_recv(fd, buffer, flags).timeout(tmot);
    return result;
}

op_aw_base 
rename(const ::std::filesystem::path& from, 
       const ::std::filesystem::path& to);

op_aw_base
renameat(const toolpex::unique_posix_fd& olddir, const ::std::filesystem::path& from, 
         const toolpex::unique_posix_fd& newdir, const ::std::filesystem::path& to, 
         int flags = 0);

op_aw_base
renameat_noreplace(const toolpex::unique_posix_fd& olddir, 
                   const ::std::filesystem::path& from, 
                   const toolpex::unique_posix_fd& newdir, 
                   const ::std::filesystem::path& to);

recv_send_aw
sendmsg(const toolpex::unique_posix_fd& fd, 
        const ::msghdr* msg, 
        int flags = 0);

recv_send_aw
sendmsg(toolpex::is_std_chrono_duration_or_time_point auto tmot, 
        const toolpex::unique_posix_fd& fd, 
        const ::msghdr* msg, 
        int flags = 0)
{
    recv_send_aw result{};
    result.batch().prep_sendmsg(fd, msg, flags).timeout(tmot);
    return result;
}

recv_send_aw
recvmsg(const toolpex::unique_posix_fd& fd, 
        ::msghdr* msg, 
        int flags = 0);

recv_send_aw
recvmsg(toolpex::is_std_chrono_duration_or_time_point auto tmot, 
        const toolpex::unique_posix_fd& fd, 
        ::msghdr* msg, 
        int flags = 0)
{
    recv_send_aw result{};
    result.batch().prep_recvmsg(fd, msg, flags).timeout(tmot);
    return result;
}

socket_aw socket(int domain, int type, int protocal, unsigned int flags = 0);

connect_aw
connect(const toolpex::unique_posix_fd& fd, 
        toolpex::ip_address::ptr addr, 
        ::in_port_t port);

connect_aw
connect(toolpex::is_std_chrono_duration_or_time_point auto tmot, 
        const toolpex::unique_posix_fd& fd, 
        toolpex::ip_address::ptr addr, 
        ::in_port_t port)
{
    connect_aw result{};
    result.batch().prep_connect(fd, ::std::move(addr), port).timeout(tmot);
    return result;
}

op_aw_base unlink(::std::filesystem::path path, int flags = 0);
op_aw_base fsync(const toolpex::unique_posix_fd& fd);

op_aw_base 
sync_file_range(const toolpex::unique_posix_fd& fd, 
                unsigned len, uint64_t offset, int flags = 0);

accept_aw
accept(toolpex::is_std_chrono_duration_or_time_point auto tmot, 
       const toolpex::unique_posix_fd& fd) noexcept
{
    accept_aw result{};
    result.batch().prep_accept(fd).timeout(tmot);
    return result;
}

} // koios::uring

#endif
