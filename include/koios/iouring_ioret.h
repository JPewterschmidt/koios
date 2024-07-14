// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_IOURING_IORET_H
#define KOIOS_IOURING_IORET_H

#include <cstdint>
#include <system_error>
#include <sys/socket.h>

#include "toolpex/unique_posix_fd.h"

namespace koios::uring
{
    struct ioret
    {
        int32_t ret{};
        uint32_t flags{};
    };

    class ioret_for_any_base : public ioret
    {
    public:
        constexpr ioret_for_any_base() noexcept = default;
        ioret_for_any_base(ioret r) noexcept;
        ioret_for_any_base(int32_t ret, uint32_t flags) noexcept
            : ioret_for_any_base{ ioret{ ret, flags } }
        {
        }

        ::std::error_code error_code() const noexcept;

    private:
        int m_errno{};
    };

    class ioret_for_data_deliver : public ioret_for_any_base
    {
    public:
        using ioret_for_any_base::ioret_for_any_base;
        size_t nbytes_delivered() const noexcept;
    };

    class ioret_for_posix_fd_result : public ioret_for_any_base
    {
    public:
        using ioret_for_any_base::ioret_for_any_base;
        ::toolpex::unique_posix_fd get_fd();
    };

    class ioret_for_socket : public ioret_for_posix_fd_result
    {
    public:
        using ioret_for_posix_fd_result::ioret_for_posix_fd_result;
        ::toolpex::unique_posix_fd get_socket_fd() { return get_fd(); }
    };

    class ioret_for_accept : public ioret_for_any_base
    {
    public:
        ioret_for_accept(
            ioret r, 
            const ::sockaddr* addr, 
        ::socklen_t len) noexcept;

        using ioret_for_any_base::ioret_for_any_base;

        ::toolpex::unique_posix_fd
        get_client();

    private:
        const ::sockaddr* m_addr{};
        const ::socklen_t m_len{};
    };

    using ioret_for_connect = ioret_for_any_base;
    using ioret_for_openat = ioret_for_posix_fd_result;

    class ioret_for_cancel : public ioret_for_any_base
    {
    public:
        using ioret_for_any_base::ioret_for_any_base;
        size_t number_canceled() const;
    };
}

#endif
