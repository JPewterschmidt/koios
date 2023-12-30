#include "koios/iouring_detials.h"

using namespace koios::uring;
using namespace koios::uring::detials;

ioret_for_data_deliver::
ioret_for_data_deliver(ioret r) noexcept 
    : ioret{ ::std::move(r) } 
{
    if (ret < 0) [[unlikely]]
    {
        m_errno = errno;
    }
}

::std::error_code ioret_for_data_deliver::
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
