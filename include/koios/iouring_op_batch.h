// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_IOURING_OP_BATCH_H
#define KOIOS_IOURING_OP_BATCH_H

#include <vector>
#include <chrono>
#include <liburing.h>
#include <filesystem>
#include <span>
#include <sys/types.h>
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
 *  Due to the lifetime issue (caused by coroutine execution) 
 *  of those objects the function arguments referenced,
 *  op_batch will allocate a bunch of memory to copy those things typically.
 *  But there's exception, those bytes content which going to be sent will not be copied.
 *  only for some peripheral data like a filesystem path string will be copied.
 *  Those copied object will last until the `op_batch` object destructed.
 *
 *  This class won't remove the `IOSQE_IO_LINK` flag, 
 *  `iouring_op_batch_execute_aw::await_suspend` will do it.
 */
class op_batch
{
public:
    op_batch(::std::pmr::memory_resource* mr = nullptr)
        : m_mr{ mr ? mr : ::std::pmr::get_default_resource() }, 
          m_rep{ m_mr }, 
          m_peripheral{ m_mr }
    {
    }

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

    /*! \brief  Prepare a POSIX accept operation of iouring.
     *
     *  \attention  There's chance that the accept iouring operation won't get return.
     *              It's somehow pretty dangours.
     *              Thus, you'd better use this with `timeout` setting.
     *
     *  \see `timeout`
     */
    op_batch& prep_accept(const toolpex::unique_posix_fd& fd) noexcept;

    /*! \brief  Prepare a POSIX write operation of iouring.
     *
     *  \param fd A open posix fd represents the writable resource.
     *
     *  \param buffer The range of bytes you want to write. 
     *                The byte contents won't been copied, 
     *                the users has to deal with the lifetime issue themselves.
     *                But this usually won't be a big issue.
     *
     *  See more at the POSIX manual pages write(2).
     */
    op_batch& prep_write(const toolpex::unique_posix_fd& fd, 
                         ::std::span<const ::std::byte> buffer, 
                         uint64_t offset = 0) noexcept;

    /*! \brief  Prepare a POSIX send operation of iouring.
     *
     *  \param fd A open posix fd represents the writable resource.
     *
     *  \param buffer The range of bytes you want to send. 
     *                The byte contents won't been copied, 
     *                the users has to deal with the lifetime issue themselves.
     *                But this usually won't be a big issue.
     *
     *  Similary to `write`, but on socket only.
     *
     *  See more at the POSIX manual pages send(2).
     */
    op_batch& prep_send(const toolpex::unique_posix_fd& fd, 
                        ::std::span<const ::std::byte> buffer, 
                        int flags = 0) noexcept;

    /*! \brief  Prepare a POSIX sendmsg operation of iouring.
     *
     *  \param fd A open posix fd represents the writable resource.
     *
     *  \param  msg A pointer point to the msghdr structure.
     *
     *  See more at the POSIX manual pages sendmsg(2).
     */
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

    /*! \brief  Prepare a POSIX recv operation of iouring.
     *
     *  \param fd A open posix fd represents the recvable resource.
     *
     *  \param buffer The buffer you want to place the bytes received. 
     *
     *  See more at the POSIX manual pages recv(2).
     */
    op_batch& prep_recv(const toolpex::unique_posix_fd& fd, 
                        ::std::span<::std::byte> buffer, 
                        int flags = 0) noexcept;

    op_batch& prep_recv(const toolpex::unique_posix_fd& fd, 
                        ::std::span<char> buffer, 
                        int flags = 0) noexcept
    {
        return prep_recv(fd, ::std::as_writable_bytes(buffer), flags);
    }

    /*! \brief  Prepare a POSIX recvmsg operation of iouring.
     *
     *  \param fd A open posix fd represents the recvable resource.
     *
     *  \param  msg A pointer point to the msghdr structure.
     *
     *  See more at the POSIX manual pages recvmsg(2).
     */
    op_batch& prep_recvmsg(const toolpex::unique_posix_fd& fd, 
                           ::msghdr* msg, 
                           int flags = 0) noexcept;

    /*! \brief  Prepare a POSIX read operation of iouring.
     *
     *  \param fd A open posix fd represents the readable resource.
     *
     *  \param buffer The buffer you want to place the bytes received. 
     *
     *  See more at the POSIX manual pages read(2).
     */
    op_batch& prep_read(const toolpex::unique_posix_fd& fd, 
                        ::std::span<::std::byte> buffer, 
                        uint64_t offset = 0) noexcept;

    op_batch& prep_read(const toolpex::unique_posix_fd& fd, 
                        ::std::span<char> buffer, 
                        uint64_t offset = 0) noexcept
    {
        return prep_read(fd, ::std::as_writable_bytes(buffer), offset);
    }

    op_batch& prep_cancel_any(const toolpex::unique_posix_fd& fd, uintptr_t userdata) noexcept;
    op_batch& prep_cancel_any(const toolpex::unique_posix_fd& fd, void* userdata) noexcept;
    op_batch& prep_cancel_any(const toolpex::unique_posix_fd& fd) noexcept;
    op_batch& prep_cancel_all(uintptr_t userdata) noexcept;
    op_batch& prep_cancel_all(void* userdata) noexcept;
    op_batch& prep_cancel_first(uintptr_t userdata) noexcept;
    op_batch& prep_cancel_first(void* userdata) noexcept;

    op_batch& prep_openat(const toolpex::unique_posix_fd& dirfd, ::std::filesystem::path p, int flags, mode_t mode);

    /*! \brief  Prepare a unlink iouring operation
     *
     *  \param path The path you want to unlink (hard). 
     *              The content of this parameter will be copied.
     *              The copied one will last until the `op_batch` destructed.
     *
     *  \return the reference to the object which this call related to.
     */
    op_batch& prep_unlink(::std::filesystem::path path, int flags = 0);

    /*! \brief  Prepare a unlinkat iouring operation
     *
     *  \param fd   The directory file descriptor.
     *  \param path The path you want to unlink (hard). 
     *              The content of this parameter will be copied.
     *              The copied one will last until the `op_batch` destructed.
     *
     *  \return the reference to the object which this call related to.
     */
    op_batch& prep_unlinkat(const toolpex::unique_posix_fd& fd, 
                            ::std::filesystem::path path, 
                            int flags = 0);

    /*! \brief  Prepare a rename iouring operation
     *
     *  \param from The original path(name) of the file you want to rename.
     *  \param to   The new name of the file you want to rename.
     *
     *  The content of those parameters will be copied.
     *  The copied ones will last until the `op_batch` destructed.
     *
     *  \return the reference to the object which this call related to.
     */
    op_batch& prep_rename(const ::std::filesystem::path& from, 
                          const ::std::filesystem::path& to);

    /*! \brief  Prepare a renameat (replace version) iouring operation
     *
     *  \param olddir  A opened fd point to the old directory of the file you want to rename.
     *  \param oldname The original path(name) of the file you want to rename.
     *  \param newdir  A opened fd point to the new directory of the file you want to rename.
     *  \param newname The new name of the file you want to rename.
     *
     *  The content of those parameters will be copied.
     *  The copied ones will last until the `op_batch` destructed.
     *
     *  \attention  If there's already a file with the same name as `newname`, it will be covered!
     *  \see `prep_renameat_noreplace`
     *
     *  \return the reference to the object which this call related to.
     */
    op_batch& prep_renameat(const toolpex::unique_posix_fd& olddir, const ::std::filesystem::path& oldname, 
                            const toolpex::unique_posix_fd& newdir, const ::std::filesystem::path& newname, 
                            int flags = 0);

    /*! \brief  Prepare a rename iouring operation
     *
     *  \param olddir  A opened fd point to the old directory of the file you want to rename.
     *  \param oldname The original path(name) of the file you want to rename.
     *  \param newdir  A opened fd point to the new directory of the file you want to rename.
     *  \param newname The new name of the file you want to rename.
     *
     *  The content of those parameters will be copied.
     *  The copied ones will last until the `op_batch` destructed.
     *
     *  If there's already a file with the same name as parame `newname`, 
     *  the operation would fail, then return a error code.
     *
     *  \see `prep_renameat`
     *
     *  \return the reference to the object which this call related to.
     */
    op_batch& prep_renameat_noreplace(const toolpex::unique_posix_fd& olddir, 
                                      const ::std::filesystem::path& oldname, 
                                      const toolpex::unique_posix_fd& newdir, 
                                      const ::std::filesystem::path& newname);

    /*! \brief  Prepare a POSIX socket iouring operation.
     *  \see the manual page of socket(2)
     *  \param domain The  domain argument specifies a communication domain.
     *  \param type The  socket  has  the indicated type, which specifies the communication semantics.
     *  \param protocal The  protocol  specifies  a  particular  protocol  to  be used with the socket.
     *  \param flags the flags bit mask you want to inform to the underlying iouring facility.
     */
    op_batch& prep_socket(int domain, int type, int protocal, unsigned int flags = 0) noexcept;
    op_batch& prep_connect(const toolpex::unique_posix_fd& fd, 
                           toolpex::ip_address::ptr addr, 
                           ::in_port_t port);

    op_batch& prep_sync_file_range(const toolpex::unique_posix_fd& fd,
                                   unsigned len, uint64_t offset, int flags = 0) noexcept;
    op_batch& prep_fsync(const toolpex::unique_posix_fd& fd) noexcept;
    op_batch& prep_fdatasync(const toolpex::unique_posix_fd& fd) noexcept;

    /*! \brief  Adding a nop sqe to the iouring event loop
     *
     *  This operation will prepare a nop sqe, could be used as test, 
     *  or as a sitimulus to reduce the sleeping time of the event loop.
     */
    op_batch& prep_nop() noexcept;

    /*! \brief  Set the timeout time point of the whole operation batch. 
     *  
     *  If there's already a timeout has been set, this function will overwrite it.
     */
    op_batch& timeout(::std::chrono::system_clock::time_point tp);

    /*! \brief  Set the timeout time duration of the whole operation batch. 
     *  
     *  If there's already a timeout has been set, this function will overwrite it.
     */
    template<typename Rep, typename Period>
    op_batch& timeout(::std::chrono::duration<Rep, Period> dura)
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

    /*! \return the reference to the representation object of this op_batch. Used by other koios uring components*/
    op_batch_rep& rep() noexcept { return m_rep; }
    const op_batch_rep& rep() const noexcept { return m_rep; }

private:
    ::std::pmr::memory_resource* m_mr{};
    op_batch_rep m_rep;
    op_peripheral m_peripheral;
};

} // namespace koios::uring

#endif
