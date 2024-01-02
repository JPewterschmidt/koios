#ifndef KOIOS_IOURING_RECV_AW_H
#define KOIOS_IOURING_RECV_AW_H

#include <system_error>
#include <cerrno>
#include <span>

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_detials.h"

#include "toolpex/unique_posix_fd.h"

namespace koios::uring
{
    class recv : public detials::iouring_aw_for_data_deliver
    {
    public:
        recv(const toolpex::unique_posix_fd& fd, 
             ::std::span<unsigned char> buffer, 
             int flags = 0);
    };
}

#endif
