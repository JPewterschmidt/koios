#include "koios/iouring_op_batch.h"
#include "toolpex/exceptions.h"

namespace koios::uring
{

task<::std::vector<ioret_for_any_base>&> 
op_batch::execute() &
{
    toolpex::not_implemented();
    co_return m_rets;
}

op_batch& op_batch::
prep_send(const toolpex::unique_posix_fd& fd, 
          ::std::span<const ::std::byte> buffer, 
          int flags) noexcept
{
    auto* cur_sqe = &m_sqes.emplace_back();
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
    auto* cur_sqe = &m_sqes.emplace_back();
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
    auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_sendmsg(cur_sqe, fd, msg, flags);
    cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_recvmsg(const toolpex::unique_posix_fd& fd, 
             ::msghdr* msg, int flags) noexcept
{
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_sendmsg(cur_sqe, fd, msg, flags);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_read(const toolpex::unique_posix_fd& fd, 
          ::std::span<::std::byte> buffer, uint64_t offset) noexcept
{
	auto* cur_sqe = &m_sqes.emplace_back();
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
	auto* cur_sqe = &m_sqes.emplace_back();
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
    struct connect_data : public peripheral_data
    {
        connect_data(::sockaddr_storage sock) noexcept
            : sock{ sock }
        {
        }

        ::sockaddr_storage sock;
    }* datap{};
    
    auto [sock, len] = addr->to_sockaddr(port);
    m_peripheral_datas.emplace_back(datap = new connect_data(sock));
	auto* cur_sqe = &m_sqes.emplace_back();
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
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_cancel64(cur_sqe, userdata, IORING_ASYNC_CANCEL_ANY); 
    cur_sqe->fd = fd;
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_any(const toolpex::unique_posix_fd& fd, 
                void* userdata) noexcept
{
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_cancel(cur_sqe, userdata, IORING_ASYNC_CANCEL_ANY); 
    cur_sqe->fd = fd;
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_any(const toolpex::unique_posix_fd& fd) noexcept
{
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_cancel(cur_sqe, 0, IORING_ASYNC_CANCEL_ANY); 
    cur_sqe->fd = fd;
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_all(uint64_t userdata) noexcept
{
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_cancel64(cur_sqe, userdata, IORING_ASYNC_CANCEL_ALL); 
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_all(void* userdata) noexcept
{
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_cancel(cur_sqe, userdata, IORING_ASYNC_CANCEL_ALL); 
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_first(uint64_t* userdata) noexcept
{
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_cancel(cur_sqe, userdata, 0); 
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_cancel_first(void* userdata) noexcept
{
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_cancel(cur_sqe, userdata, 0); 
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_unlink(const ::std::filesystem::path& path, 
            int flags) noexcept
{
    struct unlink_data : peripheral_data
    {
        unlink_data(const ::std::filesystem::path& p) noexcept : path{ p } {}
        ::std::string path;
    } *data{};

    m_peripheral_datas.emplace_back(data = new unlink_data(path));
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_unlink(cur_sqe, data->path.c_str(), flags);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

struct rename_data : public op_batch::peripheral_data
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
    rename_data* data{};
    m_peripheral_datas.emplace_back(data = new rename_data(from, to));
	auto* cur_sqe = &m_sqes.emplace_back();
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
    rename_data* data{};
    m_peripheral_datas.emplace_back(data = new rename_data(oldname, newname));
	auto* cur_sqe = &m_sqes.emplace_back();
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
    rename_data* data{};
    m_peripheral_datas.emplace_back(data = new rename_data(oldname, newname));
	auto* cur_sqe = &m_sqes.emplace_back();
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
	auto* cur_sqe = &m_sqes.emplace_back();
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
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_sync_file_range(cur_sqe, fd, len, offset, flags);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_fsync(const toolpex::unique_posix_fd& fd) noexcept
{
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_fsync(cur_sqe, fd, 0);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

op_batch& op_batch::
prep_fdatasync(const toolpex::unique_posix_fd& fd) noexcept
{
	auto* cur_sqe = &m_sqes.emplace_back();
    ::io_uring_prep_fsync(cur_sqe, fd, IORING_FSYNC_DATASYNC);
	cur_sqe->flags |= IOSQE_IO_LINK;
	return *this;
}

}
