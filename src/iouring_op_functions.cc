#include "koios/iouring_op_functions.h"

namespace koios::uring
{

op_aw_base 
rename(const ::std::filesystem::path& from, 
       const ::std::filesystem::path& to)
{
    op_aw_base result;
    result.batch().prep_rename(from, to);
    return result;
}

op_aw_base
renameat(const toolpex::unique_posix_fd& olddir, const ::std::filesystem::path& from, 
         const toolpex::unique_posix_fd& newdir, const ::std::filesystem::path& to, 
         int flags)
{
    op_aw_base result;
    result.batch().prep_renameat(olddir, from, newdir, to, flags);
    return result;
}

op_aw_base
renameat_noreplace(const toolpex::unique_posix_fd& olddir, 
                   const ::std::filesystem::path& from, 
                   const toolpex::unique_posix_fd& newdir, 
                   const ::std::filesystem::path& to)
{
    op_aw_base result;
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

op_aw_base 
unlink(::std::filesystem::path path, int flags)
{
    op_aw_base result{};
    result.batch().prep_unlink(path, flags);
    return result;
}

op_aw_base fsync(const toolpex::unique_posix_fd& fd)
{
    op_aw_base result{};
    result.batch().prep_fsync(fd);
    return result;
}

op_aw_base 
sync_file_range(const toolpex::unique_posix_fd& fd, 
                unsigned len, uint64_t offset, int flags)
{
    op_aw_base result{};
    result.batch().prep_sync_file_range(fd, len, offset, flags);
    return result;
}

connect_aw
connect(const toolpex::unique_posix_fd& fd, 
        toolpex::ip_address::ptr addr, 
        ::in_port_t port)
{
    connect_aw result{};
    result.batch().prep_connect(fd, ::std::move(addr), port);
    return result;
}

recv_send_aw
recvmsg(const toolpex::unique_posix_fd& fd, 
        ::msghdr* msg, 
        int flags)
{
    recv_send_aw result{};
    result.batch().prep_recvmsg(fd, msg, flags);
    return result;
}

recv_send_aw
sendmsg(const toolpex::unique_posix_fd& fd, 
        const ::msghdr* msg, 
        int flags)
{
    recv_send_aw result{};
    result.batch().prep_sendmsg(fd, msg, flags);
    return result;
}


} // namespace koios::uring
