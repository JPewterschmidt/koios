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

namespace koios::uring
{
    namespace detials
    {
        class ioret_for_any_base : public ioret
        {
        public:
            ioret_for_any_base(ioret r) noexcept;
            ::std::error_code error_code() const noexcept;

        private:
            int m_errno{};
        };
    }

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

    struct accepted_client
    {
        toolpex::unique_posix_fd fd;
        ::std::unique_ptr<toolpex::ip_address> ip;
    };

    class ioret_for_accept : public detials::ioret_for_any_base
    {
    public:
        ioret_for_accept(
            ioret r, 
            const ::sockaddr* addr, 
        ::socklen_t len) noexcept;

        accepted_client
        get_client();

    private:
        const ::sockaddr* m_addr{};
        const ::socklen_t m_len{};
    };

    namespace detials
    {
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
    }
}

#endif
