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
#include "koios/iouring_awaitables.h"
#include "koios/exceptions.h"
#include "koios/coroutine_mutex.h"
#include "koios/error_category.h"
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
        m_sockfd = co_await uring::bind_get_sock_tcp(m_addr, m_port);
        this->listen();
        m_waiting_latch = co_await m_waiting_queue.acquire();
        const auto& attrs = koios::get_task_scheduler().consumer_attrs();
        for (const auto& attr : attrs)
        {
            if (m_stop_src.stop_requested()) break;
            m_count.fetch_add();
            this->tcp_loop(m_stop_src.get_token(), callback).run(*attr);
        }
    }

private:
    friend class tcp_server_until_done_aw;
    lazy_task<void> tcp_loop(
        ::std::stop_token flag, 
        task_callable_concept auto userdefined) noexcept
    {
        using namespace ::std::string_literals;

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

        if (m_count.fetch_sub() <= 1)
            m_waiting_latch.unlock();

        co_return;
    }

    void listen();

private:
    toolpex::unique_posix_fd    m_sockfd;
    toolpex::ip_address::ptr    m_addr;
    ::in_port_t                 m_port;
    ::std::stop_source          m_stop_src;
    toolpex::ref_count          m_count{};

    mutable ::std::shared_mutex m_stop_related_lock;
    mutable koios::mutex        m_waiting_queue;
    koios::unique_lock<koios::mutex> m_waiting_latch;
};

namespace tcp_server_literals
{
    tcp_server operator""_tcp_s(const char* opt_ip_port, ::std::size_t len);
}

KOIOS_NAMESPACE_END

#endif
