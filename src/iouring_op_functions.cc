#include "koios/iouring_op_functions.h"

namespace koios::uring
{

normal_aw 
rename(const ::std::filesystem::path& from, 
       const ::std::filesystem::path& to)
{
    normal_aw result;
    result.batch().prep_rename(from, to);
    return result;
}

normal_aw
renameat(const toolpex::unique_posix_fd& olddir, const ::std::filesystem::path& from, 
         const toolpex::unique_posix_fd& newdir, const ::std::filesystem::path& to, 
         int flags)
{
    normal_aw result;
    result.batch().prep_renameat(olddir, from, newdir, to, flags);
    return result;
}

normal_aw
renameat_noreplace(const toolpex::unique_posix_fd& olddir, 
                   const ::std::filesystem::path& from, 
                   const toolpex::unique_posix_fd& newdir, 
                   const ::std::filesystem::path& to)
{
    normal_aw result;
    result.batch().prep_renameat_noreplace(olddir, from, newdir, to);
    return result;
}

socket_aw 
socket(int domain, int type, int protocal, unsigned int flags)
{
    socket_aw result{};
    result.batch().prep_socket(domain, type, protocal, flags);
    return result;
}

normal_aw 
unlink(::std::filesystem::path path, int flags)
{
    normal_aw result{};
    result.batch().prep_unlink(path, flags);
    return result;
}

normal_aw fsync(const toolpex::unique_posix_fd& fd)
{
    normal_aw result{};
    result.batch().prep_fsync(fd);
    return result;
}

normal_aw 
sync_file_range(const toolpex::unique_posix_fd& fd, 
                unsigned len, uint64_t offset, int flags)
{
    normal_aw result{};
    result.batch().prep_sync_file_range(fd, len, offset, flags);
    return result;
}

recv_send_aw
sendmsg(const toolpex::unique_posix_fd& fd, 
        const ::msghdr* msg, 
        int flags, 
        ::std::chrono::system_clock::time_point tmot)
{
    recv_send_aw result{};
    result.batch().prep_sendmsg(fd, msg, flags).timeout(tmot);
    return result;
}

recv_send_aw
recvmsg(const toolpex::unique_posix_fd& fd, 
        ::msghdr* msg, 
        int flags, 
        ::std::chrono::system_clock::time_point tmot)
{
    recv_send_aw result{};
    result.batch().prep_recvmsg(fd, msg, flags).timeout(tmot);
    return result;
}

connect_aw
connect(const toolpex::unique_posix_fd& fd, 
        toolpex::ip_address::ptr addr, 
        ::in_port_t port, 
        ::std::chrono::system_clock::time_point tmot)
{
    connect_aw result{};
    result.batch().prep_connect(fd, ::std::move(addr), port).timeout(tmot);
    return result;
}

accept_aw
accept(const toolpex::unique_posix_fd& fd, 
       ::std::chrono::system_clock::time_point tmot) noexcept
{
    accept_aw result{};
    result.batch().prep_accept(fd).timeout(tmot);
    return result;
}

cancel_aw cancel_any(const toolpex::unique_posix_fd& fd, uint64_t userdata)
{
    cancel_aw result{};
    if (userdata) result.batch().prep_cancel_any(fd, userdata);
    else          result.batch().prep_cancel_any(fd);
    return result;
}

cancel_aw cancel_first(uint64_t userdata)
{
    cancel_aw result{};
    result.batch().prep_cancel_first(userdata);
    return result;
}

cancel_aw cancel_all(uint64_t userdata)
{
    cancel_aw result{};
    result.batch().prep_cancel_first(userdata);
    return result;
}

} // namespace koios::uring
