#include "koios/iouring_connect_aw.h"
#include "koios/iouring_socket_aw.h"

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
                     toolpex::ip_address::ptr addr, 
                     ::in_port_t port)
        : detials::iouring_aw_for_connect(io_uring_sqe{})
    {
        auto [sock, len] = addr->to_sockaddr(port);
        m_sockaddr = sock;
        auto sqe = init_helper(fd, &m_sockaddr, len);
        *static_cast<detials::iouring_aw_for_connect*>(this) = detials::iouring_aw_for_connect(sqe);
        
        errno = 0;
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
