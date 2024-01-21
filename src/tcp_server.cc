/* koios, a c++ async runtime library.
 * copyright (c) 2024  jeremy pewterschmidt
 * 
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 * 
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 * 
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 */

#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include "toolpex/errret_thrower.h"
#include "toolpex/exceptions.h"
#include "koios/runtime.h"
#include "koios/this_task.h"
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
tcp_server(toolpex::ip_address::ptr addr, 
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

    m_sockfd = co_await uring::bind_get_sock(m_addr, m_port);
    this->listen();
    ::std::lock_guard lk{ m_futures_lock };
    const auto& attrs = koios::get_task_scheduler().consumer_attrs();
    for (const auto& attr : attrs)
    {
        m_futures.emplace_back(
            tcp_loop(m_stop_src.get_token(), aw).run_and_get_future(*attr)
        );
    }

    using namespace ::std::chrono_literals;
    co_await koios::this_task::sleep_for(500ms);
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

emitter_task<void> 
tcp_server::
tcp_loop(
    ::std::stop_token flag, 
    ::std::function<
        task<void>(toolpex::unique_posix_fd fd)
    > userdefined)
{
    using namespace ::std::string_literals;

    assert(flag.stop_possible());
    //koios::log_debug(
    //    "tcp_server start! ip: "s 
    //    + m_addr->to_string() 
    //    + ", port: "s 
    //    + ::std::to_string(m_port)
    //);
    while (!flag.stop_requested())
    {
        auto accret = co_await uring::accept(m_sockfd);
        userdefined(accret.get_client()).run();
    }
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
