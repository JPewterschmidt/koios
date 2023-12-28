#ifndef KOIOS_IOURING_READ_AW_H
#define KOIOS_IOURING_READ_AW_H

#include "koios/macros.h"
#include "koios/iouring_aw.h"

#include "toolpex/unique_posix_fd.h"

KOIOS_NAMESPACE_BEG

class iouring_read_aw : public iouring_aw
{
public:
    iouring_read_aw(const unique_posix_fd& fd, 
                    ::std::span<unsigned char> buffer, 
                    uint64_t offset = 0) 
        : iouring_aw{ init_helper(fd, buffer, offset) }
    {
    }

private:
    ::io_uring_sqe 
    init_helper(const unique_posix_fd& fd, 
                ::std::span<unsigned char> buffer, 
                uint64_t offset = 0)
    {
        ::io_uring_sqe result{};
        ::io_uring_prep_read(
            &result, fd, 
            buffer.data(), 
            buffer.size_bytes(), 
            offset
        );
        return result;
    }
};

KOIOS_NAMESPACE_END

#endif
