// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/iouring_ioret.h"
#include "koios/exceptions.h"
#include "toolpex/unique_posix_fd.h"

namespace koios::uring { 

ioret_for_any_base::
ioret_for_any_base(ioret r) noexcept 
    : ioret{ ::std::move(r) } 
{
    if (ret < 0) [[unlikely]]
    {
        if (ret == -1 && errno) m_errno = errno;
        else m_errno = -ret;
    }
}

::std::error_code ioret_for_any_base::
error_code() const noexcept
{
    if (ret >= 0) [[likely]]
        return {};
    return { m_errno, ::std::system_category() };
}

size_t 
ioret_for_data_deliver::
nbytes_delivered() const noexcept
{
    return ret >= 0 ? static_cast<size_t>(ret) : 0;
}

::toolpex::unique_posix_fd 
ioret_for_posix_fd_result::
get_fd()
{
    if (auto ec = this->error_code(); ec)
    {
        throw koios::socket_exception{ ::std::move(ec) };
    }
    return { ret };
}

ioret_for_accept::
ioret_for_accept(ioret r, const ::sockaddr* addr, ::socklen_t len) noexcept
    : ioret_for_any_base{ r }, m_addr{ addr }, m_len{ len }
{
}

::toolpex::unique_posix_fd
ioret_for_accept::
get_client()
{
    if (auto ec = this->error_code(); ec)
        throw koios::uring_exception(ec);
    return { ret };
}

size_t 
ioret_for_cancel::number_canceled() const
{
    if (auto ec = this->error_code(); ec)
        throw uring_exception{ ec };
    return this->ret;
}

} // namespace koios::uring
