#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include "toolpex/errret_thrower.h"
#include "toolpex/exceptions.h"
#include "koios/runtime.h"
#include <cassert>
#include <netinet/in.h>
#include <thread>

KOIOS_NAMESPACE_BEG

using namespace toolpex;

tcp_server_until_done_aw::
tcp_server_until_done_aw(tcp_server& server)
    : m_server{ server }
{
}

bool 
tcp_server_until_done_aw::
await_ready() const noexcept 
{ 
    return m_server.is_stop(); 
}

void 
tcp_server_until_done_aw::
await_suspend(task_on_the_fly h)
{
    m_server.add_waiting(::std::move(h));
}

tcp_server::
tcp_server(::std::unique_ptr<toolpex::ip_address> addr, 
           ::in_port_t port)
    : m_addr{ ::std::move(addr) }, m_port{ port }
{
}

task<void> 
tcp_server::
start(::std::function<task<void>(toolpex::unique_posix_fd)> aw)
{
    bool expected{ true };
    if (!m_stop.compare_exchange_strong(expected, false))
        co_return;

    co_await this->bind();
    this->listen();
    auto& schr = get_task_scheduler();
    const auto& attrs = schr.consumer_attrs();
    ::std::lock_guard lk{ m_futures_lock };
    for (const auto& attr : attrs)
    {
        m_futures.emplace_back(
            tcp_loop(m_stop_src.get_token(), aw).run_and_get_future(*attr)
        );
    }

    co_return;
}

void
tcp_server::
until_stop_blk()
{
    ::std::lock_guard lk{ m_futures_lock };
    for (auto& f : m_futures)
    {
        f.get();
    }
}

task<void> 
tcp_server::
tcp_loop(
    ::std::stop_token flag, 
    ::std::function<
        task<void>(toolpex::unique_posix_fd fd)
    > userdefined)
{
    using namespace ::std::string_literals;

    assert(flag.stop_possible());
    koios::log_debug("tcp_server start! ip: "s + m_addr->to_string() + ", port: "s + ::std::to_string(m_port));
    while (!flag.stop_requested())
    {
        auto accret = co_await uring::accept(m_sockfd);
        userdefined(accret.get_client()).run();
    }
    co_return;
}

task<void> tcp_server::bind()
{
    auto sockret = co_await uring::socket(AF_INET, SOCK_STREAM, 0);
    if (auto ec = sockret.error_code(); ec)
    {
        throw toolpex::posix_exception{ ec };
    }
    m_sockfd = sockret.get_socket_fd();

    toolpex::errret_thrower et{};

    const int true_value{ 1 };
    const ::socklen_t vallen{ sizeof(true_value) };
    et << ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEPORT, &true_value, vallen);
    et << ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &true_value, vallen);

    const auto [addr, size] = m_addr->to_sockaddr(m_port);

    ::sockaddr_in sock_tmp{};
    ::std::memcpy(&sock_tmp, &addr, size);

    et << ::bind(
        m_sockfd, 
        reinterpret_cast<const ::sockaddr*>(&addr), 
        size
    );

    co_return;
}

void tcp_server::listen()
{
    toolpex::errret_thrower et{};
    et << ::listen(m_sockfd, 4096);
}

void tcp_server::stop()
{
    m_stop.store(true);
    m_sockfd = toolpex::unique_posix_fd{};

    ::std::lock_guard lk{ m_waitings_lock };
    for (auto& h : m_waitings)
    {
        get_task_scheduler().enqueue(::std::move(h));
    }
}

KOIOS_NAMESPACE_END
