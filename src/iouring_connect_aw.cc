#include "koios/iouring_connect_aw.h"

namespace koios::uring
{
    static ::io_uring_sqe 
    init_helper(const toolpex::unique_posix_fd& fd, 
                ::std::unique_ptr<toolpex::ip_address> addr, 
                ::in_port_t port)
    {
        ::io_uring_sqe result{};
        auto [saddr, size] = addr->to_sockaddr(port);
        ::io_uring_prep_connect(
            &result, fd, 
            reinterpret_cast<sockaddr*>(&saddr), 
            size
        ); 
        return result;
    }

    connect::connect(const toolpex::unique_posix_fd& fd, 
                     ::std::unique_ptr<toolpex::ip_address> addr, 
                     ::in_port_t port)
        : detials::iouring_aw_for_connect(init_helper(fd, ::std::move(addr), port))
    {
        errno = 0;
    }
}
