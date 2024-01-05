#ifndef KOIOS_IOURING_CONNECT_AW_H
#define KOIOS_IOURING_CONNECT_AW_H

#include "koios/macros.h"
#include "toolpex/ipaddress.h"
#include "koios/iouring_aw.h"
#include "toolpex/unique_posix_fd.h"

namespace koios::uring
{
    class connect : public iouring_aw
    {
    public:
        connect(const toolpex::unique_posix_fd& fd, 
                ::std::unique_ptr<toolpex::ip_address> addr, 
                ::in_port_t port);
    };
}

#endif
