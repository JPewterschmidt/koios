#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include "toolpex/errret_thrower.h"
#include "toolpex/exceptions.h"
#include "koios/runtime.h"
#include <cassert>

KOIOS_NAMESPACE_BEG

using namespace toolpex;

tcp_server::
tcp_server(::std::unique_ptr<toolpex::ip_address> addr, 
           ::in_port_t port, 
           ::std::function<task<void>(uring::accepted_client)> aw)
    : m_addr{ ::std::move(addr) }, m_port{ port }
{
    bind().result();
    listen();
    auto& schr = get_task_scheduler();
    const auto& attrs = schr.consumer_attrs();
    ::std::lock_guard lk{ m_futures_lock };
    for (const auto& attr : attrs)
    {
        m_futures.emplace_back(
            tcp_loop(m_stop_src.get_token(), aw).run_and_get_future(*attr)
        );
    }
}

void
tcp_server::
until_stop()
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
        task<void>(uring::accepted_client)
    > userdefined)
{
    assert(flag.stop_possible());
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
    const auto addr = m_addr->to_sockaddr(m_port);
    et << ::bind(
        m_sockfd, 
        reinterpret_cast<const ::sockaddr*>(&addr), 
        sizeof(addr)
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
    m_sockfd = toolpex::unique_posix_fd{};
}

KOIOS_NAMESPACE_END
