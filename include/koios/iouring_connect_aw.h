#ifndef KOIOS_IOURING_CONNECT_AW_H
#define KOIOS_IOURING_CONNECT_AW_H

#include "koios/macros.h"
#include "toolpex/ipaddress.h"
#include "koios/iouring_aw.h"
#include "toolpex/unique_posix_fd.h"
#include "koios/iouring_detials.h"
#include "koios/task.h"

namespace koios::uring
{
    class connect : public detials::iouring_aw_for_connect
    {
    public:
        connect(const toolpex::unique_posix_fd& fd, 
                ::std::unique_ptr<toolpex::ip_address> addr, 
                ::in_port_t port);

    private:
        ::sockaddr_storage m_sockaddr{};
    };

    ::koios::task<toolpex::unique_posix_fd> 
    connect_get_sock(::std::unique_ptr<toolpex::ip_address> addr, 
                     ::in_port_t port, 
                     unsigned int socket_flags = 0);
}
#endif
