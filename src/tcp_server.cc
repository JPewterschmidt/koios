// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include "toolpex/exceptions.h"
#include "koios/runtime.h"
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
    const auto port  = str.substr(delima + 1);
    const auto ipstr = str.substr(0, delima);
    
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
      m_addr{::std::move(other.m_addr)},
      m_port{::std::move(other.m_port)},
      m_stop_src{::std::move(other.m_stop_src)}
{
}

void
tcp_server::
until_done_blk()
{
    this->until_done_async().result();
}

bool tcp_server::is_stop() const noexcept 
{ 
    if (m_stop_src.stop_requested())
    {
        return m_wait_stop.ready();
    }
    return false;
}

void tcp_server::stop()
{
    m_stop_src.request_stop();
}

task<> tcp_server::until_done_async()
{
    if (this->is_stop()) co_return;
    co_await m_wait_stop.wait();
    co_return;
}

KOIOS_NAMESPACE_END
