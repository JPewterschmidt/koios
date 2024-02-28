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

#ifndef KOIOS_IOURING_OP_BATCH_H
#define KOIOS_IOURING_OP_BATCH_H

#include <vector>
#include <chrono>
#include <liburing.h>
#include <filesystem>
#include <span>
#include "toolpex/unique_posix_fd.h"
#include "toolpex/ipaddress.h"
#include "koios/traits.h"
#include "koios/task.h"
#include "koios/iouring_ioret.h"
#include "koios/iouring_op_batch_rep.h"
#include "koios/iouring_op_peripheral.h"
#include "koios/iouring_op_batch_execute_aw.h"

namespace koios::uring
{

/*! \brief The linked iouring operations initialization helper
 *
 *  Due to the iouring supports linked cqe to process series of cqes with in a FIFO order. 
 *  We provide the koios binding of it to support it.
 *  You can call those member functions to generate corresponding cqes.
 *  This class object guarantees that the operation order.
 *  Those preparation information will be stored in a `iouring_op_batch_rep` object, 
 *  When you call `co_await execute()` the `iouring_op_batch_rep` object 
 *  will be passed to the iouring event loop related faciality.
 *
 *  This class won't remove the `IOSQE_IO_LINK` flag, 
 *  `iouring_op_batch_execute_aw::await_suspend` will do it.
 */
class op_batch
{
public:
    constexpr op_batch() noexcept = default;
    op_batch(op_batch&&) noexcept = default;
    op_batch& operator=(op_batch&&) noexcept = default;

    /*! \brief  This function generates a awaitable object 
     *          contains the reference of `iouring_op_batch_rep`.
     *
     *  `op_batch_execute_aw::await_suspend()` will remove the `IOSQE_IO_LINK` flag of the last op.
     *  to break the linked operation chain.
     *
     *  \return A awaitable object which registers this series of operations.
     */
    op_batch_execute_aw execute() & noexcept;

    op_batch& prep_accept(const toolpex::unique_posix_fd& fd) noexcept;

    op_batch& prep_write(const toolpex::unique_posix_fd& fd, 
                         ::std::span<const ::std::byte> buffer, 
                         uint64_t offset = 0) noexcept;

    op_batch& prep_send(const toolpex::unique_posix_fd& fd, 
                        ::std::span<const ::std::byte> buffer, 
                        int flags = 0) noexcept;

    op_batch& prep_sendmsg(const toolpex::unique_posix_fd& fd, 
                           const ::msghdr* msg, 
                           int flags = 0) noexcept;

    op_batch& prep_write(const toolpex::unique_posix_fd& fd, 
                         ::std::span<const char> buffer, 
                         uint64_t offset = 0) noexcept
    {
        return prep_write(fd, ::std::as_bytes(buffer), offset);
    }

    op_batch& prep_send(const toolpex::unique_posix_fd& fd, 
                        ::std::span<const char> buffer, 
                        int flags = 0) noexcept
    {
        return prep_send(fd, ::std::as_bytes(buffer), flags);
    }

    op_batch& prep_recv(const toolpex::unique_posix_fd& fd, 
                        ::std::span<::std::byte> buffer, 
                        int flags = 0) noexcept;

    op_batch& prep_recv(const toolpex::unique_posix_fd& fd, 
                        ::std::span<char> buffer, 
                        int flags = 0) noexcept
    {
        return prep_recv(fd, ::std::as_writable_bytes(buffer), flags);
    }

    op_batch& prep_recvmsg(const toolpex::unique_posix_fd& fd, 
                           ::msghdr* msg, 
                           int flags = 0) noexcept;

    op_batch& prep_read(const toolpex::unique_posix_fd& fd, 
                        ::std::span<::std::byte> buffer, 
                        uint64_t offset = 0) noexcept;

    op_batch& prep_read(const toolpex::unique_posix_fd& fd, 
                        ::std::span<char> buffer, 
                        uint64_t offset = 0) noexcept
    {
        return prep_read(fd, ::std::as_writable_bytes(buffer), offset);
    }

    op_batch& prep_cancel_any(const toolpex::unique_posix_fd& fd, uint64_t userdata) noexcept;
    op_batch& prep_cancel_any(const toolpex::unique_posix_fd& fd, void* userdata) noexcept;
    op_batch& prep_cancel_any(const toolpex::unique_posix_fd& fd) noexcept;
    op_batch& prep_cancel_all(uint64_t userdata) noexcept;
    op_batch& prep_cancel_all(void* userdata) noexcept;
    op_batch& prep_cancel_first(uint64_t userdata) noexcept;
    op_batch& prep_cancel_first(void* userdata) noexcept;

    op_batch& prep_unlink(const ::std::filesystem::path& path, 
                          int flags = 0) noexcept;

    op_batch& prep_rename(const ::std::filesystem::path& from, 
                          const ::std::filesystem::path& to) noexcept;

    op_batch& prep_renameat(const toolpex::unique_posix_fd& olddir, const ::std::filesystem::path& oldname, 
                            const toolpex::unique_posix_fd& newdir, const ::std::filesystem::path& newname, 
                            int flags = 0) noexcept;

    op_batch& prep_renameat_noreplace(const toolpex::unique_posix_fd& olddir, 
                                      const ::std::filesystem::path& oldname, 
                                      const toolpex::unique_posix_fd& newdir, 
                                      const ::std::filesystem::path& newname) noexcept;

    op_batch& prep_socket(int domain, int type, int protocal, unsigned int flags = 0) noexcept;
    op_batch& prep_connect(const toolpex::unique_posix_fd& fd, 
                           toolpex::ip_address::ptr addr, 
                           ::in_port_t port) noexcept;

    op_batch& prep_sync_file_range(const toolpex::unique_posix_fd& fd,
                                   unsigned len, uint64_t offset, int flags = 0) noexcept;
    op_batch& prep_fsync(const toolpex::unique_posix_fd& fd) noexcept;
    op_batch& prep_fdatasync(const toolpex::unique_posix_fd& fd) noexcept;
    op_batch& prep_nop() noexcept;

    /*! \brief  Set the timeout time point of the whole operation batch. 
     *  
     *  If there's already a timeout has been set, this function will overwrite it.
     */
    op_batch& timeout(::std::chrono::system_clock::time_point tp) noexcept;

    /*! \brief  Set the timeout time duration of the whole operation batch. 
     *  
     *  If there's already a timeout has been set, this function will overwrite it.
     */
    template<typename Rep, typename Period>
    op_batch& timeout(::std::chrono::duration<Rep, Period> dura) noexcept
    {
        if (dura == decltype(dura)::max()) return *this;
        return timeout(dura + ::std::chrono::system_clock::now());
    }

    op_batch& clear() noexcept { m_rep.clear(); return *this; }

    /*! \brief  Check if all the io operations has successfully return. 
     *
     *  except for the timeout setting of the batch if there is one.
     */
    [[nodiscard]] bool all_success() const noexcept;

    /*! \brief  Check wether user has set a timeout. */
    bool was_timeout_set() const noexcept{ return m_rep.was_timeout_set(); };

    /*! \brief  Check wether the batch has been canceled by timeout or not. 
     *  
     *  This function will checkout the last return slots.
     */
    bool is_timeout() const noexcept;

    /*! \return return the `error_code` object of the timeout operation (the last one). */
    ::std::error_code timeout_req_ec() const noexcept;

    op_batch_rep& rep() noexcept { return m_rep; }
    const op_batch_rep& rep() const noexcept { return m_rep; }

private:
    op_batch_rep m_rep;
    op_peripheral m_peripheral;
};

} // namespace koios::uring

#endif
