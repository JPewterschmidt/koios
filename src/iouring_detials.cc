#include "koios/iouring_detials.h"
#include "koios/exceptions.h"
#include "toolpex/unique_posix_fd.h"

using namespace koios::uring;
using namespace koios::uring::detials;

ioret_for_any_base::
ioret_for_any_base(ioret r) noexcept 
    : ioret{ ::std::move(r) } 
{
    if (ret < 0) [[unlikely]]
    {
        m_errno = errno;
    }
}

::std::error_code ioret_for_any_base::
error_code() const noexcept
{
    if (ret >= 0) [[likely]]
        return {};
    return { m_errno, ::std::system_category() };
}

ioret_for_data_deliver
iouring_aw_for_data_deliver::
await_resume()
{
    return { iouring_aw::await_resume() };
}


ioret_for_data_deliver::
ioret_for_data_deliver(ioret r) noexcept
    : detials::ioret_for_any_base{ ::std::move(r) }
{
}

size_t 
ioret_for_data_deliver::
nbytes_delivered() const noexcept
{
    return ret >= 0 ? static_cast<size_t>(ret) : 0;
}

ioret_for_socket::
ioret_for_socket(ioret r) noexcept
    : detials::ioret_for_any_base{ ::std::move(r) }
{
}

::toolpex::unique_posix_fd 
ioret_for_socket::
get_socket_fd()
{
    if (auto ec = this->error_code(); ec)
    {
        throw koios::socket_exception{ ::std::move(ec) };
    }
    return { ret };
}


ioret_for_socket 
iouring_aw_for_socket::
await_resume()
{
    return { iouring_aw::await_resume() };
}
