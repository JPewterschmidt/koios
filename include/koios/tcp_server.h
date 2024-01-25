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
#include "toolpex/ref_count.h"
#include "koios/iouring_awaitables.h"
#include "koios/exceptions.h"
#include "koios/coroutine_mutex.h"
#include <functional>
#include <memory>
#include <stop_token>
#include <mutex>

KOIOS_NAMESPACE_BEG

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
     *
     *  \attention This function could be called only once !
     */
    task<void> start(task_callable_concept auto callback)
    {
        m_sockfd = co_await uring::bind_get_sock(m_addr, m_port);
        this->listen();
        m_waiting_latch = co_await m_waiting_queue.acquire();
        ::std::lock_guard lk{ m_futures_lock };
        const auto& attrs = koios::get_task_scheduler().consumer_attrs();
        for (const auto& attr : attrs)
        {
            ++m_count;
            m_futures.emplace_back(
                tcp_loop(m_stop_src.get_token(), callback).run_and_get_future(*attr)
            );
        }

        co_return;
    }

    void stop();
    void until_stop_blk();
    bool is_stop() const noexcept 
    { 
        if (m_stop_src.stop_requested())
        {
            ::std::unique_lock lk{ m_futures_lock };
            return m_count == 0;
        }
        return false;
    }
    task<> until_stop_async();

private:
    friend class tcp_server_until_done_aw;
    emitter_task<void> tcp_loop(
        ::std::stop_token flag, 
        task_callable_concept auto userdefined) noexcept
    {
        using namespace ::std::string_literals;

        assert(flag.stop_possible());
        while (!flag.stop_requested())
        try
        {
            auto accret = co_await uring::accept(m_sockfd); // to prevent so called 
                                                            // "insufficient contextual information to determine type
            make_emitter(userdefined, accret.get_client()).run();
        }
        catch (const koios::exception& e)
        {
            e.log();
        }
        catch (...)
        {
            koios::log_error("tcp_server catched unknow exception");
        }

        ::std::unique_lock lk{ m_futures_lock };
        if (--m_count <= 0)
        {
            m_waiting_latch.unlock();
        }

        co_return;
    }

    void listen();

private:
    toolpex::unique_posix_fd m_sockfd;
    toolpex::ip_address::ptr m_addr;
    ::in_port_t m_port;
    ::std::stop_source m_stop_src;
    int m_count{};

    ::std::vector<koios::future<void>> m_futures;
    mutable ::std::mutex m_futures_lock;
    mutable koios::mutex m_waiting_queue;
    koios::unique_lock m_waiting_latch;
};

KOIOS_NAMESPACE_END

#endif
