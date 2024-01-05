#include "koios/iouring_detials.h"
#include "koios/exceptions.h"
#include "toolpex/unique_posix_fd.h"

namespace koios::uring { 

::std::ostream& operator<<(
    ::std::ostream& os, 
    const accepted_client& client)
{
    os << "fd: " << client.fd << ", ip: " << client.ip->to_string();
    return os;
}

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

static 
::io_uring_sqe
init_helper_for_accept_aw(const toolpex::unique_posix_fd& fd, int flags, 
                          ::sockaddr_storage* addr, ::socklen_t* len) noexcept
{
    ::io_uring_sqe result{};
    ::io_uring_prep_accept(
        &result, fd, 
        reinterpret_cast<::sockaddr*>(addr), len, 
        flags
    );
    return result;
}

detials::iouring_aw_for_accept::
iouring_aw_for_accept(const toolpex::unique_posix_fd& fd, int flags) noexcept
    : iouring_aw{ init_helper_for_accept_aw(fd, flags, &m_ss, &m_len) }
{
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

accepted_client
ioret_for_accept::
get_client()
{
    return { 
        toolpex::unique_posix_fd{ ret }, 
        toolpex::ip_address::make(m_addr, m_len)
    };
}

ioret_for_connect
detials::iouring_aw_for_connect::
await_resume()
{
    return iouring_aw::await_resume();
}

} // namespace koios::uring
