/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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

    class ioret_for_socket : public ioret_for_any_base
    {
    public:
        using ioret_for_any_base::ioret_for_any_base;
        ::toolpex::unique_posix_fd get_socket_fd();       
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

    class ioret_for_cancel : public ioret_for_any_base
    {
    public:
        using ioret_for_any_base::ioret_for_any_base;
        size_t number_canceled() const;
    };
}

#endif
