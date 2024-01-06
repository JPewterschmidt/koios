#ifndef KOIOS_IOURING_WRITE_AW_H
#define KOIOS_IOURING_WRITE_AW_H

#include <system_error>
#include <cerrno>
#include <string_view>

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_detials.h"

#include "toolpex/unique_posix_fd.h"

KOIOS_NAMESPACE_BEG

namespace uring
{
    class write : public detials::iouring_aw_for_data_deliver
    {
    public:
        write(const toolpex::unique_posix_fd& fd, 
              ::std::span<const unsigned char> buffer, 
              uint64_t offset = 0);

        write(const toolpex::unique_posix_fd& fd, 
              ::std::span<const ::std::byte> buffer, 
              uint64_t offset = 0);

        write(const toolpex::unique_posix_fd& fd, 
              ::std::span<const char> buffer, 
              uint64_t offset = 0);

        write(const toolpex::unique_posix_fd& fd, 
              ::std::string_view buffer,
              uint64_t offset = 0);
    };
}

KOIOS_NAMESPACE_END

#endif
