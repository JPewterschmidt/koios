// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/iouring_op_batch.h"
#include "toolpex/exceptions.h"
#include "toolpex/assert.h"
#include "toolpex/convert_to_systime.h"

namespace koios::uring
{

op_batch_execute_aw 
op_batch::
execute() & noexcept 
{ 
    toolpex_assert(!m_rep.empty());
    return { m_rep };
}

struct sock_data
{
    sock_data(::sockaddr_storage sock) noexcept
        : sock{ sock }
    {
    }

    sock_data(sock_data&&) noexcept = default;

    ::sockaddr_storage sock;
};

op_batch& op_batch::
prep_accept(const toolpex::unique_posix_fd& fd) noexcept
{
    auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_accept(cur_sqe, fd, nullptr, nullptr, 0);
    cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_send(const toolpex::unique_posix_fd& fd, 
          ::std::span<const ::std::byte> buffer, 
          int flags) noexcept
{
    auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_send(
        cur_sqe, fd, 
        buffer.data(), static_cast<size_t>(buffer.size_bytes()), 
        flags
    );
    cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_recv(const toolpex::unique_posix_fd& fd, 
          ::std::span<::std::byte> buffer, int flags) noexcept
{
    auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_recv(
        cur_sqe, fd, 
        buffer.data(), static_cast<size_t>(buffer.size_bytes()), 
        flags
    );
    cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_sendmsg(const toolpex::unique_posix_fd& fd, 
             const ::msghdr* msg, int flags) noexcept
{
    auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_sendmsg(cur_sqe, fd, msg, flags);
    cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_recvmsg(const toolpex::unique_posix_fd& fd, 
             ::msghdr* msg, int flags) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_recvmsg(cur_sqe, fd, msg, flags);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_read(const toolpex::unique_posix_fd& fd, 
          ::std::span<::std::byte> buffer, uint64_t offset) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_read(
        cur_sqe, fd, 
        buffer.data(), static_cast<unsigned int>(buffer.size_bytes()), 
        offset
    );
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_write(const toolpex::unique_posix_fd& fd, 
           ::std::span<const ::std::byte> buffer, uint64_t offset) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_write(
        cur_sqe, fd, 
        buffer.data(), static_cast<unsigned int>(buffer.size_bytes()), 
        offset
    );
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_connect(const toolpex::unique_posix_fd& fd, 
             toolpex::ip_address::ptr addr, ::in_port_t port)
{
    auto [sock, len] = addr->to_sockaddr(port);
    sock_data* datap{m_peripheral.add<sock_data>(sock)};

	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_connect(
        cur_sqe, fd, 
        reinterpret_cast<sockaddr*>(&datap->sock), 
        len
    );
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_any(const toolpex::unique_posix_fd& fd, 
                uintptr_t userdata) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_cancel64(cur_sqe, userdata, IORING_ASYNC_CANCEL_ANY); 
    cur_sqe->fd = fd;
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_any(const toolpex::unique_posix_fd& fd, 
                void* userdata) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_cancel(cur_sqe, userdata, IORING_ASYNC_CANCEL_ANY); 
    cur_sqe->fd = fd;
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_any(const toolpex::unique_posix_fd& fd) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_cancel(cur_sqe, 0, IORING_ASYNC_CANCEL_ANY); 
    cur_sqe->fd = fd;
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_all(uintptr_t userdata) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_cancel64(cur_sqe, userdata, IORING_ASYNC_CANCEL_ALL); 
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_all(void* userdata) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_cancel(cur_sqe, userdata, IORING_ASYNC_CANCEL_ALL); 
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_first(uintptr_t userdata) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_cancel64(cur_sqe, userdata, 0); 
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_first(void* userdata) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_cancel(cur_sqe, userdata, 0); 
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

struct path_data 
{
    path_data(const ::std::filesystem::path& p) noexcept : path{ p } {}
    path_data(path_data&&) noexcept = default;
    ::std::string path;
};

op_batch& op_batch::
prep_openat(const toolpex::unique_posix_fd& dirfd, 
            ::std::filesystem::path p, int flags, mode_t m)
{
    path_data* data{m_peripheral.add<path_data>(::std::move(p))};
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_openat(cur_sqe, dirfd, data->path.c_str(), flags, m);
	cur_sqe->flags |= IOSQE_IO_LINK;

    return *this;
}

op_batch& op_batch::
prep_unlink(::std::filesystem::path path, 
            int flags)
{
    path_data* data{m_peripheral.add<path_data>(::std::move(path))};

	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_unlink(cur_sqe, data->path.c_str(), flags);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_unlinkat(const toolpex::unique_posix_fd& fd,
              ::std::filesystem::path path, 
              int flags)
{
    path_data* data{m_peripheral.add<path_data>(::std::move(path))};

	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_unlinkat(cur_sqe, fd, data->path.c_str(), flags);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

struct rename_data 
{
    rename_data(const auto&f, const auto& t) noexcept
        : from{ f }, to{ t }
    {
    }
    rename_data(rename_data&&) noexcept = default;

    ::std::string from, to;
};

op_batch& op_batch::
prep_rename(const ::std::filesystem::path& from, 
            const ::std::filesystem::path& to) 
{
    auto* data = m_peripheral.add<rename_data>(from, to);
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_rename(cur_sqe, data->from.c_str(), data->to.c_str());
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_renameat(const toolpex::unique_posix_fd& olddir, 
              const ::std::filesystem::path& oldname, 
              const toolpex::unique_posix_fd& newdir, 
              const ::std::filesystem::path& newname, 
              int flags)
{
    auto* data = m_peripheral.add<rename_data>(oldname, newname);
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_renameat(
        cur_sqe, 
        olddir, data->from.c_str(), 
        newdir, data->to.c_str(), 
        flags
    );
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_renameat_noreplace(const toolpex::unique_posix_fd& olddir, 
                        const ::std::filesystem::path& oldname, 
                        const toolpex::unique_posix_fd& newdir, 
                        const ::std::filesystem::path& newname)
{
    auto* data = m_peripheral.add<rename_data>(oldname, newname);
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_renameat(
        cur_sqe, 
        olddir, data->from.c_str(), 
        newdir, data->to.c_str(), 
        RENAME_NOREPLACE
    );
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_socket(int domain, int type, 
            int protocal, unsigned int flags) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_socket(cur_sqe, domain, type, protocal, flags);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_sync_file_range(const toolpex::unique_posix_fd& fd, 
                     unsigned len, 
                     uint64_t offset, 
                     int flags) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_sync_file_range(cur_sqe, fd, len, offset, flags);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_fsync(const toolpex::unique_posix_fd& fd) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_fsync(cur_sqe, fd, 0);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_fdatasync(const toolpex::unique_posix_fd& fd) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_fsync(cur_sqe, fd, IORING_FSYNC_DATASYNC);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_nop() noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_nop(cur_sqe);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
timeout(::std::chrono::system_clock::time_point tp)
{
    if (tp == ::std::chrono::system_clock::time_point::max())
        return *this;

    ::io_uring_sqe* cur_sqe{ 
        m_rep.was_timeout_set() ? 
            &m_rep.back() :m_rep.get_sqe()
    };

    struct timeout_data     
    {
        timeout_data(::std::chrono::system_clock::time_point const& tp) noexcept 
            : ts{ toolpex::convert_to_timespec<__kernel_timespec>(tp) } 
        {
        }

        timeout_data(timeout_data&&) noexcept = default;

        __kernel_timespec ts;
    } *data{ m_peripheral.add<timeout_data>(tp) };

    ::io_uring_prep_link_timeout(
        cur_sqe, 
        &data->ts, 
        IORING_TIMEOUT_REALTIME | IORING_TIMEOUT_ABS
    );
    
    m_rep.set_timeout();
    return *this;
}

bool op_batch::all_success() const noexcept
{
    const auto& rets = m_rep.return_slots();
    for (const auto& ret : rets | ::std::ranges::views::take(rets.size() - was_timeout_set()))
    {
        if (ret.error_code()) return false;
    }
    return true;
}

bool op_batch::is_timeout() const noexcept
{
    if (!was_timeout_set()) return false;
    return m_rep
        .return_slots()
        .back()
        .error_code()
        .value() != ECANCELED;
}

::std::error_code op_batch::timeout_req_ec() const noexcept
{
    if (!was_timeout_set()) return {};
    return m_rep.return_slots().back().error_code();
}

} // namespace koios::uring
