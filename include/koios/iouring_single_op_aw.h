/* Koios, A c++ async runtime library.
 * Copyright (C) 2023  Jeremy Pewterschmidt
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

#ifndef KOIOS_IOURING_SINGLE_OP_AW_H
#define KOIOS_IOURING_SINGLE_OP_AW_H

#include "koios/iouring_ioret.h"
#include "koios/iouring_op_batch.h"

namespace koios::uring
{
    class op_aw_base
    {
    public:
        auto& batch() { return m_batch; }

    protected:
        template<typename IoRetT> IoRetT resume_template() noexcept
        {
            return ::std::move(this->batch().rep().return_slots().front());
        }

    private:
        op_batch m_batch;
    };

    class nop_aw : public op_aw_base, public op_batch_execute_aw
    {
    public:
        nop_aw() noexcept;
    };

    class normal_aw : public op_aw_base, public op_batch_execute_aw
    {
    public:
        normal_aw() noexcept;
        ioret_for_any_base await_resume() noexcept
        {
            return this->resume_template<ioret_for_any_base>();
        }
    };

    class posix_result_aw : public op_aw_base, public op_batch_execute_aw
    {
    public:
        posix_result_aw() noexcept;
        ioret_for_posix_fd_result await_resume() noexcept
        {
            return this->resume_template<ioret_for_posix_fd_result>();
        }
    };

    class read_write_aw : public op_aw_base, public op_batch_execute_aw
    {
    public:
        read_write_aw() noexcept;
        ioret_for_data_deliver await_resume() noexcept 
        { 
            return this->resume_template<ioret_for_data_deliver>(); 
        }
    };

    using recv_send_aw = read_write_aw;

    class socket_aw : public op_aw_base, public op_batch_execute_aw
    {
    public:
        socket_aw() noexcept;
        ioret_for_socket await_resume() noexcept
        { 
            return this->resume_template<ioret_for_socket>(); 
        }
    };

    class accept_aw : public op_aw_base, public op_batch_execute_aw
    {
    public:
        accept_aw() noexcept;
        ioret_for_accept await_resume() noexcept
        { 
            return this->resume_template<ioret_for_accept>(); 
        }
    };

    class connect_aw : public op_aw_base, public op_batch_execute_aw
    {
    public:
        connect_aw() noexcept;
        ioret_for_any_base await_resume() noexcept
        { 
            return this->resume_template<ioret_for_any_base>(); 
        }
    };

    class cancel_aw : public op_aw_base, public op_batch_execute_aw
    {
    public:
        cancel_aw() noexcept;
        ioret_for_cancel await_resume() noexcept
        {
            return this->resume_template<ioret_for_cancel>(); 
        }
    };

    class fsync_aw : public op_aw_base, public op_batch_execute_aw
    {
    public:
        fsync_aw() noexcept;
        ioret_for_any_base await_resume() noexcept
        {
            return this->resume_template<ioret_for_cancel>();
        }
    };

} // namespace koios::uring

#endif
