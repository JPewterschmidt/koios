// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_TCP_SERVER_H
#define KOIOS_TCP_SERVER_H

#include "koios/macros.h"
#include "koios/task.h"
#include "koios/this_task.h"
#include "koios/functional.h"
#include "toolpex/ipaddress.h"
#include "toolpex/ref_count.h"
#include "toolpex/errret_thrower.h"
#include "koios/iouring_awaitables.h"
#include "koios/exceptions.h"
#include "koios/coroutine_mutex.h"
#include "koios/error_category.h"
#include "koios/wait_group.h"
#include <functional>
#include <memory>
#include <stop_token>
#include <mutex>
#include <shared_mutex>

KOIOS_NAMESPACE_BEG

class tcp_server : public toolpex::move_only
{
public:
    tcp_server(toolpex::ip_address::ptr addr, 
               ::in_port_t port);

    tcp_server(::in_port_t port)
        : tcp_server{ toolpex::ipv4_address::get_allzero(), port }
    {
    }

    tcp_server(tcp_server&& other) noexcept;
    tcp_server& operator=(tcp_server&& other) noexcept;

    void stop();
    void until_done_blk();
    bool is_stop() const noexcept;
    task<> until_done_async();

    /*! \brief  Make the tcp server start working
     *
     *  \param  callback a copyable callable object, 
     *          receive a `toolpex::unique_posix_fd` 
     *          as it's unique parameter.
     *
     *  \attention This function could be called only once !
     */
    task<> start(task_callable_concept auto callback)
    {
        // This call will set port and address reuse.
        m_sockfd = co_await uring::bind_get_sock_tcp(m_addr, m_port);
        toolpex::errret_thrower{} << ::listen(m_sockfd, 4096);
        const auto& attrs = koios::get_task_scheduler().consumer_attrs();
        for (const auto& attr : attrs)
        {
            this->tcp_loop(m_stop_src.get_token(), callback).run(*attr);
        }
    }

private:
    lazy_task<void> tcp_loop(
        ::std::stop_token flag, 
        task_callable_concept auto userdefined) noexcept
    {
        using namespace ::std::string_literals;

        wait_group_guard wg_handler{ m_wait_stop };
        while (!flag.stop_requested())
        {
            using namespace ::std::chrono_literals;
            auto accret = co_await uring::accept(m_sockfd, 50ms);
            if (auto ec = accret.error_code(); 
                is_timeout_ec(ec))
            {
                continue;
            }
            else if (ec)
            {
                koios::log_error(ec.message());
                continue;
            }
            else 
            {
                auto client = accret.get_client();
                make_lazy(userdefined, ::std::move(client)).run();
            }
        }

        co_return;
    }

private:
    toolpex::unique_posix_fd    m_sockfd;
    toolpex::ip_address::ptr    m_addr;
    ::in_port_t                 m_port;
    ::std::stop_source          m_stop_src;
    wait_group                  m_wait_stop;
};

namespace tcp_server_literals
{
    tcp_server operator""_tcp_s(const char* opt_ip_port, ::std::size_t len);
}

KOIOS_NAMESPACE_END

#endif
