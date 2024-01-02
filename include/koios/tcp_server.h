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

class tcp_server
{
public:
    tcp_server(::std::unique_ptr<toolpex::ip_address> addr, 
               ::in_port_t port, 
               ::std::function<task<void>(uring::accepted_client)> aw);

    void stop();
    void until_stop();

private:
    task<void> tcp_loop(
        ::std::stop_token flag, 
        ::std::function<task<void>(uring::accepted_client)> userdefined);

    task<void> bind();
    void listen();

private:
    toolpex::unique_posix_fd m_sockfd;
    ::std::unique_ptr<toolpex::ip_address> m_addr;
    ::in_port_t m_port;
    ::std::stop_source m_stop_src;
    ::std::vector<koios::future<void>> m_futures;
    mutable ::std::mutex m_futures_lock;
};

KOIOS_NAMESPACE_END

#endif
