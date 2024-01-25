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

tcp_server::
tcp_server(toolpex::ip_address::ptr addr, 
           ::in_port_t port)
    : m_addr{ ::std::move(addr) }, m_port{ port }
{
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

void tcp_server::listen()
{
    toolpex::errret_thrower et{};
    et << ::listen(m_sockfd, 4096);
}

void tcp_server::stop()
{
    m_stop_src.request_stop();
    m_sockfd = toolpex::unique_posix_fd{};
}

task<> tcp_server::until_stop_async()
{
    if (is_stop()) co_return;
    [[maybe_unused]] auto lk = co_await m_waiting_queue.acquire();
    co_return;
}

KOIOS_NAMESPACE_END
