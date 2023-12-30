#ifndef KOIOS_IOURING_READ_AW_H
#define KOIOS_IOURING_READ_AW_H

#include <system_error>
#include <cerrno>
#include <span>

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_detials.h"

#include "toolpex/unique_posix_fd.h"

KOIOS_NAMESPACE_BEG

namespace uring
{
    class read : public detials::iouring_aw_for_data_deliver
    {
    public:
        read(const toolpex::unique_posix_fd& fd, 
             ::std::span<unsigned char> buffer, 
             uint64_t offset = 0);
    };
}

KOIOS_NAMESPACE_END

#endif
