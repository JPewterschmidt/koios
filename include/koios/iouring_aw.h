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

#ifndef KOIOS_IOURING_AW_H
#define KOIOS_IOURING_AW_H

#include "koios/macros.h"
#include "koios/iouring_ioret.h"
#include "koios/task_on_the_fly.h"
#include <liburing.h>
#include <memory>
#include <cstdint>
#include <cstddef>
#include <span>

namespace koios::uring
{
    class iouring_aw
    {
    public:
        iouring_aw() = default;
        iouring_aw(::io_uring_sqe sqe);
        iouring_aw(iouring_aw&&) noexcept = default;
        iouring_aw& operator=(iouring_aw&&) noexcept = default;
        auto* sqe_ptr() noexcept { return &m_sqe; }
        constexpr bool await_ready() const noexcept { return false; }
        void await_suspend(task_on_the_fly h);
        detials::ioret_for_any_base await_resume() { return *m_ret; }

    private:
        ::std::shared_ptr<ioret> m_ret{::std::make_shared<ioret>()};
        ::io_uring_sqe m_sqe{};
    };
}

#endif
