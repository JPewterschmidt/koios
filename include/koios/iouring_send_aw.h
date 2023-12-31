#ifndef KOIOS_IOURING_SEND_AW_H
#define KOIOS_IOURING_SEND_AW_H

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_detials.h"

#include "toolpex/unique_posix_fd.h"
#include <cstddef>

namespace koios::uring
{
    class send : public detials::iouring_aw_for_data_deliver
    {
    public:
        send(const toolpex::unique_posix_fd& fd, 
             ::std::span<const unsigned char> buffer, 
             int flags = 0);

        send(const toolpex::unique_posix_fd& fd, 
             ::std::string_view sv, 
             int flags = 0);

        send(const toolpex::unique_posix_fd& fd, 
             ::std::span<const ::std::byte> buffer, 
             int flags = 0);
    };
}

#endif
