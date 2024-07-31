// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_IOURING_SINGLE_OP_AW_H
#define KOIOS_IOURING_SINGLE_OP_AW_H

#include <memory>
#include <memory_resource>

#include "koios/iouring_ioret.h"
#include "koios/iouring_op_batch.h"

namespace koios::uring
{
    class op_aw_base
    {
    public:
        op_aw_base() 
            : m_mbr{ ::std::make_unique<::std::pmr::monotonic_buffer_resource>() }, 
              m_batch(m_mbr.get())
        {
        }
        
        op_aw_base(op_aw_base&&) noexcept = default;
        op_aw_base& operator=(op_aw_base&&) noexcept = default;

        auto& batch() { return m_batch; }

    protected:
        template<typename IoRetT> IoRetT resume_template() noexcept
        {
            return ::std::move(this->batch().rep().return_slots().front());
        }

    private:
        ::std::unique_ptr<::std::pmr::monotonic_buffer_resource> m_mbr;
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
