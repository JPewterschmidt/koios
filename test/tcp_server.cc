#include "gtest/gtest.h"
#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include <memory>

using namespace koios;
using namespace toolpex::ip_address_literals;
using namespace ::std::string_view_literals;
using namespace ::std::chrono_literals;

namespace
{
    ::std::unique_ptr<tcp_server> sp{};
    ::std::atomic_bool flag{ false };

    lazy_task<void> tcp_server_app(toolpex::unique_posix_fd client) noexcept
    try
    {
        ::std::string msg = "nice to meet you!!!!";
        ::std::array<char, 128> buffer{};

        const auto recv_ret = co_await uring::recv(client, buffer);

        co_await uring::send(client, msg);
        ::std::string_view sv{ buffer.data(), recv_ret.nbytes_delivered() };
        if (sv.contains("stop"))
        {
            flag.store(true), sp->stop();
        }

        co_return;
    }
    catch (...)
    {
        co_return;
    }

    task<void> client_app() noexcept
    try
    {
        auto sock = co_await uring::connect_get_sock("::1"_ip, 8890);
        co_await uring::send(sock, "nice to meet you, and stop."sv);
        co_await uring::send(sock, "nice to meet you, and stop."sv);
        co_await uring::send(sock, "nice to meet you, and stop."sv);
        co_await uring::send(sock, "nice to meet you, and stop."sv);
        co_await uring::send(sock, "nice to meet you, and stop."sv);
        co_await uring::send(sock, "nice to meet you, and stop."sv);
        co_await uring::send(sock, "nice to meet you, and stop."sv);

        co_return;
    }
    catch (...)
    {
        co_return;
    }

    lazy_task<bool> emit_basic_test()
    {
        sp.reset(new tcp_server("::1"_ip, 8890));

        co_await sp->start(tcp_server_app);
        co_await client_app(); 
        co_await sp->until_done_async();

        sp = nullptr;
        co_return flag;
    }

    task<bool> mute_client_app()
    {
        auto sock = co_await uring::connect_get_sock("::1"_ip, 8890);
        ::std::array<::std::byte, 4> buffer{};

        ::std::error_code ec;
        co_await uring::recv_fill_buffer(sock, buffer, 0, ec, 100ms);

        if (buffer[0] == ::std::byte{}) co_return false;
        co_return true;
    }

    task<bool> recv_timeout_server(toolpex::unique_posix_fd client) 
    {
        ::std::array<::std::byte, 128> buffer{};
        size_t recved = co_await uring::recv_fill_buffer(client, buffer, 0, 10ms);
        if (recved == 0) 
        {
            co_await uring::send(client, "nice to meet you");
            co_return true;
        }
        co_return false;
    }

    lazy_task<bool> emit_recv_timeout_test()
    {
        sp.reset(new tcp_server("::1"_ip, 8890));
        co_await sp->start(recv_timeout_server);

        if (!co_await mute_client_app())
        {
            sp->stop();
            co_await sp->until_done_async();
            sp = nullptr;
            ::std::cout << "shit1" << ::std::endl;
            co_return false;
        }
        sp->stop();
        co_await sp->until_done_async();
        sp = nullptr;
        co_return true;
    }
}

TEST(tcp_server, basic)
{
    ASSERT_TRUE(emit_basic_test().result());
}

TEST(tcp_server, recv_timeout)
{
    ASSERT_TRUE(emit_recv_timeout_test().result());
}
