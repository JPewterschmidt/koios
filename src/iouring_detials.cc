/* koios, a c++ async runtime library.
 * copyright (c) 2024  jeremy pewterschmidt
 * 
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 * 
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 * 
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 */

#include "koios/iouring_detials.h"
#include "koios/exceptions.h"
#include "toolpex/unique_posix_fd.h"

namespace koios::uring { 

detials::ioret_for_any_base::
ioret_for_any_base(ioret r) noexcept 
    : ioret{ ::std::move(r) } 
{
    if (ret < 0) [[unlikely]]
    {
        if (errno != 0)
            m_errno = errno;
        else m_errno = -ret;
    }
}

::std::error_code detials::ioret_for_any_base::
error_code() const noexcept
{
    if (ret >= 0) [[likely]]
        return {};
    return { m_errno, ::std::system_category() };
}

ioret_for_data_deliver
detials::iouring_aw_for_data_deliver::
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
detials::iouring_aw_for_socket::
await_resume()
{
    return { iouring_aw::await_resume() };
}

ioret_for_accept::
ioret_for_accept(ioret r, const ::sockaddr* addr, ::socklen_t len) noexcept
    : detials::ioret_for_any_base{ r }, m_addr{ addr }, m_len{ len }
{
}

detials::iouring_aw_for_accept::
iouring_aw_for_accept(const toolpex::unique_posix_fd& fd, int flags) noexcept
{
    ::io_uring_prep_accept(
        this->sqe_ptr(), fd, 
        reinterpret_cast<::sockaddr*>(&m_ss), &m_len, 
        flags
    );
}

ioret_for_accept
detials::iouring_aw_for_accept::
await_resume()
{
    return { 
        iouring_aw::await_resume(), 
        reinterpret_cast<::sockaddr*>(&m_ss), 
        m_len
    };
}

::toolpex::unique_posix_fd
ioret_for_accept::
get_client()
{
    if (auto ec = this->error_code(); ec)
        throw koios::uring_exception(ec);
    return { ret };
}

ioret_for_connect
detials::iouring_aw_for_connect::
await_resume()
{
    return iouring_aw::await_resume();
}

size_t 
ioret_for_cancel::number_canceled() const
{
    if (auto ec = this->error_code(); ec)
        throw uring_exception{ ec };
    return this->ret;
}

ioret_for_cancel
detials::iouring_aw_for_cancel::
await_resume()
{
    return iouring_aw::await_resume();
}

} // namespace koios::uring
