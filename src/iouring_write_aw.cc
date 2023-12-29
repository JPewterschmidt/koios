#include <cerrno>
#include "koios/iouring_write_aw.h"

using namespace koios::io;

ioret_for_writing::
ioret_for_writing(ioret r) noexcept
    : ioret{ ::std::move(r) }
{
    if (ret < 0) [[unlikely]]
    {
        m_errno = errno;
    }
}

::std::error_code
ioret_for_writing::
error_code() const noexcept
{
    if (ret >= 0) [[likely]]
        return {};
    return { m_errno, ::std::system_category() };
}

static 
::io_uring_sqe
init_helper(const toolpex::unique_posix_fd& fd, 
            ::std::span<const unsigned char> buffer, 
            uint64_t offset)
{
    ::io_uring_sqe result{};
    ::io_uring_prep_write(
        &result, fd, 
        buffer.data(), 
        static_cast<unsigned int>(buffer.size_bytes()), 
        offset
    );
    return result;
}

write::write(const toolpex::unique_posix_fd& fd, 
             ::std::span<const unsigned char> buffer, 
             uint64_t offset)
    : iouring_aw{ init_helper(fd, buffer, offset) }
{
    errno = 0;
}

ioret_for_writing
write::await_resume()
{
    return { iouring_aw::await_resume() };
}
