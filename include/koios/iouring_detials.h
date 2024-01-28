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

#ifndef KOIOS_IOURING_DETIALS_H
#define KOIOS_IOURING_DETIALS_H

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_ioret.h"
#include "toolpex/unique_posix_fd.h"
#include "toolpex/ipaddress.h"
#include <cstdint>
#include <cstddef>
#include <system_error>
#include <utility>
#include <iostream>

namespace koios::uring
{
    class ioret_for_data_deliver : public detials::ioret_for_any_base
    {
    public:
        ioret_for_data_deliver(ioret r) noexcept;
        size_t nbytes_delivered() const noexcept;
    };

    class ioret_for_socket : public detials::ioret_for_any_base
    {
    public:
        ioret_for_socket(ioret r) noexcept;
        ::toolpex::unique_posix_fd get_socket_fd();       
    };

    class ioret_for_accept : public detials::ioret_for_any_base
    {
    public:
        ioret_for_accept(
            ioret r, 
            const ::sockaddr* addr, 
        ::socklen_t len) noexcept;

        ::toolpex::unique_posix_fd
        get_client();

    private:
        const ::sockaddr* m_addr{};
        const ::socklen_t m_len{};
    };

    using ioret_for_connect = typename detials::ioret_for_any_base;

    class ioret_for_cancel : public detials::ioret_for_any_base
    {
    public:
        using detials::ioret_for_any_base::ioret_for_any_base;
        size_t number_canceled() const;
    };

    namespace detials
    {
        class iouring_aw_for_connect : public iouring_aw
        {
        public:
            template<typename... Args>
            iouring_aw_for_connect(Args&&... args)
                : iouring_aw(::std::forward<Args>(args)...)
            {
            }

            ioret_for_connect await_resume();
        };

        class iouring_aw_for_data_deliver : public iouring_aw
        {
        public:
            template<typename... Args>
            iouring_aw_for_data_deliver(Args&&... args)
                : iouring_aw(::std::forward<Args>(args)...)
            {
            }

            ioret_for_data_deliver await_resume();
        };

        class iouring_aw_for_socket : public iouring_aw
        {
        public:
            template<typename... Args>
            iouring_aw_for_socket(Args&&... args)
                : iouring_aw(::std::forward<Args>(args)...)
            {
            }
            
            ioret_for_socket await_resume();
        };

        class iouring_aw_for_accept : public iouring_aw
        {
        public:
            iouring_aw_for_accept(const toolpex::unique_posix_fd& fd, int flags = 0) noexcept;
            ioret_for_accept await_resume();

        private:
            ::sockaddr_storage m_ss{};
            ::socklen_t m_len{};
        };

        class iouring_aw_for_cancel : public iouring_aw
        {
        public:
            using iouring_aw::iouring_aw;           
            ioret_for_cancel await_resume();           
        };
    }
}

#endif
