#include <cassert>
#include "koios/iouring_op_batch.h"
#include "toolpex/exceptions.h"
#include "toolpex/convert_to_systime.h"
#include "toolpex/bits_manipulator.h"

namespace koios::uring
{

op_batch_execute_aw 
op_batch::
execute() & noexcept 
{ 
    assert(!m_rep.empty());
    auto& last_sqe = m_rep.back();
    last_sqe.flags = toolpex::bits_manipulator{last_sqe.flags}
       .remove(static_cast<uint8_t>(IOSQE_IO_LINK));
    return { m_rep };
}

struct sock_data : public op_peripheral::data_interface
{
    sock_data(::sockaddr_storage sock) noexcept
        : sock{ sock }
    {
    }

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
    ::io_uring_prep_send(
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
    ::io_uring_prep_sendmsg(cur_sqe, fd, msg, flags);
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
             toolpex::ip_address::ptr addr, ::in_port_t port) noexcept
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
                uint64_t userdata) noexcept
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
prep_cancel_all(uint64_t userdata) noexcept
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
prep_cancel_first(uint64_t* userdata) noexcept
{
	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_cancel(cur_sqe, userdata, 0); 
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

op_batch& op_batch::
prep_unlink(const ::std::filesystem::path& path, 
            int flags) noexcept
{
    struct unlink_data : op_peripheral::data_interface
    {
        unlink_data(const ::std::filesystem::path& p) noexcept : path{ p } {}
        ::std::string path;
    } *data{m_peripheral.add<unlink_data>(path)};

	auto* cur_sqe = m_rep.get_sqe();
    ::io_uring_prep_unlink(cur_sqe, data->path.c_str(), flags);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

struct rename_data : public op_peripheral::data_interface
{
    rename_data(const auto&f, const auto& t) noexcept
        : from{ f }, to{ t }
    {
    }

    ::std::string from, to;
};

op_batch& op_batch::
prep_rename(const ::std::filesystem::path& from, 
            const ::std::filesystem::path& to) noexcept
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
              int flags) noexcept
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
                        const ::std::filesystem::path& newname) noexcept
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
timeout(::std::chrono::system_clock::time_point tp) noexcept
{
    assert(!m_rep.empty());

    auto* cur_sqe = m_rep.get_sqe();
    struct timeout_data : public op_peripheral::data_interface
    {
        timeout_data(::std::chrono::system_clock::time_point const& tp) noexcept 
            : ts{ toolpex::convert_to_timespec<__kernel_timespec>(tp) } 
        {
        }

        __kernel_timespec ts;
    } *data{ m_peripheral.add<timeout_data>(tp) };

    ::io_uring_prep_link_timeout(
        cur_sqe, 
        &data->ts, 
        IORING_TIMEOUT_REALTIME | IORING_TIMEOUT_ABS
    );
    
    m_was_timeout_set = true;
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
    return all_success() && m_rep
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
