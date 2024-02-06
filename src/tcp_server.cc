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
#include <string_view>
#include <algorithm>

KOIOS_NAMESPACE_BEG

using namespace toolpex;

namespace tcp_server_literals
{

tcp_server operator""_tcp_s(const char* opt_ip_port, ::std::size_t len)
{
    const ::std::string_view str{ opt_ip_port, len };
    const auto delima = str.rfind(':');
    if (delima == ::std::string_view::npos) // without port
        throw koios::exception{"tcp_server literals: port is required."};
    const auto port  = str.substr(0, delima);
    const auto ipstr = str.substr(delima + 1);
    
    const auto portnum = ::atoi(port.data());
    return { ip_address::make(ipstr), static_cast<::in_port_t>(portnum) };
}

}

tcp_server::
tcp_server(toolpex::ip_address::ptr addr, 
           ::in_port_t port)
    : m_addr{ ::std::move(addr) }, m_port{ port }
{
}

tcp_server::
tcp_server(tcp_server&& other) noexcept
    : m_sockfd{ ::std::move(other.m_sockfd) },
      m_addr{::std::move(m_addr)},
      m_port{::std::move(m_port)},
      m_stop_src{::std::move(m_stop_src)},
      m_loop_handles{::std::move(m_loop_handles)},
      m_stop_complete{::std::move(m_stop_complete)}
{
}

tcp_server::~tcp_server() noexcept
try
{
    m_stop_complete.get();
}
catch (const koios::exception& e)
{
    e.log();
}
catch (const ::std::exception& e)
{
    koios::log_error(e.what());
}
catch (...)
{
    koios::log_error("unknow exception was catched during the destruction of tcp_server");
}

void
tcp_server::
until_stop_blk()
{
    until_stop_async().result();
}

bool tcp_server::is_stop() const noexcept 
{ 
    if (m_stop_src.stop_requested())
    {
        return m_count.load() == 0;
    }
    return false;
}

void tcp_server::listen()
{
    toolpex::errret_thrower et{};
    et << ::listen(m_sockfd, 4096);
}

void tcp_server::stop()
{
    //if (m_stop_src.stop_requested()) return;
    m_stop_src.request_stop();
    m_stop_complete = send_cancel_to_awaiting_accept().run_and_get_future();
    m_sockfd = toolpex::unique_posix_fd{};
}

task<> tcp_server::until_stop_async()
{
    if (is_stop()) co_return;
    [[maybe_unused]] auto lk = co_await m_waiting_queue.acquire();
    co_return;
}

task<> tcp_server::send_cancel_to_awaiting_accept() const noexcept
{
    ::std::shared_lock lk{ m_stop_related_lock };
    for (void* handle : m_loop_handles)
    {
        lk.unlock();
        co_await uring::cancel_all(handle);
        lk.lock();
    }
    co_return;
}

KOIOS_NAMESPACE_END
