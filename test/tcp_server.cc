#include "gtest/gtest.h"
#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"

using namespace koios;

namespace
{
    tcp_server* sp{};
    ::std::atomic_bool flag{ false };

    task<void> tcp_server_app(toolpex::unique_posix_fd client)
    {
        auto [addr, port] = toolpex::ip_address::getpeername(client);

        ::std::string msg = "fuck you!!!!";

        ::std::array<char, 128> buffer{};

        const auto recv_ret = co_await uring::recv(client, buffer);

        co_await uring::send(client, msg);
        ::std::string_view sv{ buffer.data(), recv_ret.nbytes_delivered() };
        if (sv.contains("stop") && sp)
            flag.store(true), sp->stop();

        co_return;
    }

    task<void> client_app()
    {
        using namespace toolpex::ip_address_literals;
        using namespace ::std::string_view_literals;

        auto sock = co_await uring::connect_get_sock("::1"_ip, 8890);
        co_await uring::send(sock, "fuck you, and stop."sv);
        co_await uring::send(sock, "fuck you, and stop."sv);
        co_await uring::send(sock, "fuck you, and stop."sv);
        co_await uring::send(sock, "fuck you, and stop."sv);
        co_await uring::send(sock, "fuck you, and stop."sv);
        co_await uring::send(sock, "fuck you, and stop."sv);

        co_return;
    }

    emitter_task<bool> emit_test()
    {
        using namespace toolpex::ip_address_literals;
        using namespace ::std::string_view_literals;

        tcp_server server("::1"_ip, 8890);
        co_await server.start(tcp_server_app);
        sp = &server;
        co_await client_app();
        co_await client_app();
        co_await client_app();
        co_await client_app();
        co_await client_app();

        co_await server.until_stop_async();
        
        co_return flag;
    }
}

TEST(tcp_server, basic)
{
    ASSERT_TRUE(emit_test().result());
}
