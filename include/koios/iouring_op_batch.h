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

class op_batch
{
public:
    constexpr op_batch() = default;
    op_batch(op_batch&&) noexcept = default;
    op_batch& operator=(op_batch&&) noexcept = default;

    op_batch_execute_aw execute() & noexcept;

    op_batch& prep_send(const toolpex::unique_posix_fd& fd, 
                        ::std::span<const ::std::byte> buffer, 
                        int flags = 0) noexcept;

    op_batch& prep_recv(const toolpex::unique_posix_fd& fd, 
                        ::std::span<::std::byte> buffer, 
                        int flags = 0) noexcept;

    op_batch& prep_sendmsg(const toolpex::unique_posix_fd& fd, 
                           const ::msghdr* msg, 
                           int flags = 0) noexcept;

    op_batch& prep_recvmsg(const toolpex::unique_posix_fd& fd, 
                           ::msghdr* msg, 
                           int flags = 0) noexcept;

    op_batch& prep_read(const toolpex::unique_posix_fd& fd, 
                        ::std::span<::std::byte> buffer, 
                        uint64_t offset = 0) noexcept;

    op_batch& prep_write(const toolpex::unique_posix_fd& fd, 
                         ::std::span<const ::std::byte> buffer, 
                         uint64_t offset = 0) noexcept;

    op_batch& prep_cancel_any(const toolpex::unique_posix_fd& fd, uint64_t userdata) noexcept;
    op_batch& prep_cancel_any(const toolpex::unique_posix_fd& fd, void* userdata) noexcept;
    op_batch& prep_cancel_any(const toolpex::unique_posix_fd& fd) noexcept;
    op_batch& prep_cancel_all(uint64_t userdata) noexcept;
    op_batch& prep_cancel_all(void* userdata) noexcept;
    op_batch& prep_cancel_first(uint64_t* userdata) noexcept;
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
    op_batch& set_timeout(::std::chrono::system_clock::time_point timeout) noexcept;

    template<typename Rep, typename Period>
    op_batch& set_timeout(::std::chrono::duration<Rep, Period> timeout) noexcept
    {
        return set_timeout(timeout + ::std::chrono::system_clock::now());
    }

    op_batch& clear() noexcept { m_rep.clear(); return *this; }


private:
    op_batch_rep& rep() noexcept { return m_rep; }
    const op_batch_rep& rep() const noexcept { return m_rep; }

private:
    op_batch_rep m_rep;
    op_peripheral m_peripheral;
};

}

#endif
