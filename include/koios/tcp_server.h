#ifndef KOIOS_TCP_SERVER_H
#define KOIOS_TCP_SERVER_H

#include "koios/macros.h"
#include "koios/task.h"
#include "toolpex/ipaddress.h"
#include "koios/iouring_socket_aw.h"
#include <functional>
#include <memory>
#include <stop_token>
#include <mutex>

KOIOS_NAMESPACE_BEG

class tcp_server;
class tcp_server_until_done_aw
{
public:
    tcp_server_until_done_aw(tcp_server& server);
    
    bool await_ready() const noexcept;
    void await_suspend(task_on_the_fly h);
    constexpr void await_resume() const noexcept {};

private:
    tcp_server& m_server;
};

class tcp_server
{
public:
    tcp_server(::std::unique_ptr<toolpex::ip_address> addr, 
               ::in_port_t port, 
               ::std::function<task<void>(uring::accepted_client)> aw);

    void stop();
    void until_stop_blk();
    bool is_stop() const { return m_stop.load(); }
    tcp_server_until_done_aw until_stop_async() { return { *this }; }

private:
    friend class tcp_server_until_done_aw;
    task<void> tcp_loop(
        ::std::stop_token flag, 
        ::std::function<task<void>(uring::accepted_client)> userdefined);

    task<void> bind();
    void listen();
    void add_waiting(task_on_the_fly h)
    {
        ::std::lock_guard lk{ m_waitings_lock };
        m_waitings.emplace_back(::std::move(h));
    }

private:
    ::std::atomic_bool m_stop{ false };

    toolpex::unique_posix_fd m_sockfd;
    ::std::unique_ptr<toolpex::ip_address> m_addr;
    ::in_port_t m_port;
    ::std::stop_source m_stop_src;

    ::std::vector<koios::future<void>> m_futures;
    mutable ::std::mutex m_futures_lock;

    ::std::vector<task_on_the_fly> m_waitings;
    mutable ::std::mutex m_waitings_lock;
};

KOIOS_NAMESPACE_END

#endif
