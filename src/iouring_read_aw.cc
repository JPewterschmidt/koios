#include "koios/iouring_read_aw.h"
#include <system_error>
#include <liburing.h>

using namespace koios;

io::ioret_for_reading::
ioret_for_reading(ioret r) noexcept 
    : ioret{ ::std::move(r) } 
{
    if (ret < 0) [[unlikely]]
    {
        m_errno = errno;
    }
}

::std::error_code io::ioret_for_reading::
error_code() const noexcept
{
    if (ret >= 0) [[likely]]
        return {};
    return { m_errno, ::std::system_category() };
}

static ::io_uring_sqe 
init_helper(const toolpex::unique_posix_fd& fd, 
            ::std::span<unsigned char> buffer, 
            uint64_t offset = 0)
{
    ::io_uring_sqe result{};
    ::io_uring_prep_read(
        &result, fd, 
        buffer.data(), 
        static_cast<unsigned int>(buffer.size_bytes()), 
        offset
    );
    return result;
}

io::read::read(const toolpex::unique_posix_fd& fd, 
               ::std::span<unsigned char> buffer, 
               uint64_t offset)
    : iouring_aw{ init_helper(fd, buffer, offset) }
{
    errno = 0;
}

io::ioret_for_reading
io::read::
await_resume() 
{
    return { iouring_aw::await_resume() };
}
