#include "koios/iouring_connect_aw.h"

namespace koios::uring
{
    static ::io_uring_sqe 
    init_helper(const toolpex::unique_posix_fd& fd, 
                ::sockaddr_storage* saddr, ::socklen_t size)
    {
        ::io_uring_sqe result{};
        ::io_uring_prep_connect(
            &result, fd, 
            reinterpret_cast<sockaddr*>(saddr), 
            size
        ); 
        return result;
    }

    connect::connect(const toolpex::unique_posix_fd& fd, 
                     ::std::unique_ptr<toolpex::ip_address> addr, 
                     ::in_port_t port)
        : detials::iouring_aw_for_connect(io_uring_sqe{})
    {
        auto [sock, len] = addr->to_sockaddr(port);
        m_sockaddr = sock;
        auto sqe = init_helper(fd, &m_sockaddr, len);
        *static_cast<detials::iouring_aw_for_connect*>(this) = detials::iouring_aw_for_connect(sqe);
        
        errno = 0;
    }
}
