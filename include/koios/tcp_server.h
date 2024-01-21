/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KOIOS_TCP_SERVER_H
#define KOIOS_TCP_SERVER_H

#include "koios/macros.h"
#include "koios/task.h"
#include "koios/this_task.h"
#include "koios/functional.h"
#include "toolpex/ipaddress.h"
#include "koios/iouring_awaitables.h"
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
    tcp_server(toolpex::ip_address::ptr addr, 
               ::in_port_t port);

    tcp_server(::in_port_t port)
        : tcp_server{ toolpex::ipv4_address::get_allzero(), port }
    {
    }

    /*! \brief  Make the tcp server start working
     *
     *  \param  callback a copyable callable object, 
     *          receive a `toolpex::unique_posix_fd` 
     *          as it's unique parameter.
     */
    task<void> start(task_callable_concept auto callback)
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
                tcp_loop(m_stop_src.get_token(), callback).run_and_get_future(*attr)
            );
        }

        using namespace ::std::chrono_literals;
        co_await koios::this_task::sleep_for(500ms);
        co_return;
    }

    void stop();
    void until_stop_blk();
    bool is_stop() const { return m_stop.load(); }
    tcp_server_until_done_aw until_stop_async() { return { *this }; }

private:
    friend class tcp_server_until_done_aw;
    emitter_task<void> tcp_loop(
        ::std::stop_token flag, 
        task_callable_concept auto userdefined)
    {
        using namespace ::std::string_literals;

        assert(flag.stop_possible());
        while (!flag.stop_requested())
        {
            auto accret = co_await uring::accept(m_sockfd);
            make_emitter(userdefined, accret.get_client()).run();
        }
        co_return;
    }

    void listen();
    void add_waiting(task_on_the_fly h)
    {
        ::std::lock_guard lk{ m_waitings_lock };
        m_waitings.emplace_back(::std::move(h));
    }

private:
    ::std::atomic_bool m_stop{ true };

    toolpex::unique_posix_fd m_sockfd;
    toolpex::ip_address::ptr m_addr;
    ::in_port_t m_port;
    ::std::stop_source m_stop_src;

    ::std::vector<koios::future<void>> m_futures;
    mutable ::std::mutex m_futures_lock;

    ::std::vector<task_on_the_fly> m_waitings;
    mutable ::std::mutex m_waitings_lock;
};

KOIOS_NAMESPACE_END

#endif
