#include "gtest/gtest.h"
#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include <memory>

using namespace koios;

namespace
{
    ::std::unique_ptr<tcp_server> sp{};
    ::std::atomic_bool flag{ false };

    emitter_task<void> tcp_server_app(toolpex::unique_posix_fd client) noexcept
    try
    {
        ::std::string msg = "fuck you!!!!";
        ::std::array<char, 128> buffer{};

        const auto recv_ret = co_await uring::recv(client, buffer);

        co_await uring::send(client, msg);
        ::std::string_view sv{ buffer.data(), recv_ret.nbytes_delivered() };
        if (sv.contains("stop"))
            flag.store(true), sp->stop();

        co_return;
    }
    catch (...)
    {
        co_return;
    }

    task<void> client_app() noexcept
    try
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
        co_await uring::send(sock, "fuck you, and stop."sv);

        co_return;
    }
    catch (...)
    {
        co_return;
    }

    emitter_task<bool> emit_test()
    {
        using namespace toolpex::ip_address_literals;
        using namespace ::std::string_view_literals;
        using namespace ::std::chrono_literals;

        sp.reset(new tcp_server("::1"_ip, 8890));
        co_await sp->start(tcp_server_app);

        for (size_t i{}; i < 20; ++i)
            co_await client_app(); // some of this never return, and caused memory leak, 
                                   // how can I make sure that this `client_app` know 
                                   // it has a caller even those the caller handler was not been set
                                   // in time.
                                   //
                                   // ioruing_event_loop take the credit. It should not to be work-stealing

        co_await sp->until_stop_async();
        co_return flag;
    }
}

TEST(tcp_server, basic)
{
    ASSERT_TRUE(emit_test().result());
}
