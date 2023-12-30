#ifndef KOIOS_IOURING_SOCKET_AW_H
#define KOIOS_IOURING_SOCKET_AW_H

#include <system_error>
#include <cerrno>
#include <span>

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_detials.h"

#include "toolpex/unique_posix_fd.h"

#include <sys/socket.h>

namespace koios::uring
{
    class socket : public detials::iouring_aw_for_socket
    {
    public:
        socket(int domain, int type, int protocal, unsigned int flags);
    };
}

#endif
