#include "gtest/gtest.h"
#include "koios/iouring_awaitables.h"
#include "toolpex/errret_thrower.h"

#include <ranges>
#include <string_view>
#include <memory>

using namespace koios;
using namespace ::std::chrono_literals;
using namespace ::std::string_view_literals;
using namespace toolpex::ip_address_literals;

namespace
{

::std::string g_file_name{ "testfile_for_iouring.txt" };
task<toolpex::unique_posix_fd> create_file()
{
    toolpex::errret_thrower et;
    co_return et << ::creat(g_file_name.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
}

task<toolpex::unique_posix_fd> open_file()
{
    toolpex::errret_thrower et;
    co_return et << ::open(g_file_name.c_str(), O_RDWR);
}

task<> delete_file()
{
    co_await uring::unlink(g_file_name);
}

emitter_task<bool> append_all_test()
{
    auto content = "123456789"sv;
    ::std::array<char, 5> buffer{};

    co_await delete_file();
    // Created by `::creat` could not as the reading source
    auto fd = co_await create_file();
    
    co_await uring::append_all(fd, content);

    ::std::error_code ec;
    co_await uring::append_all(fd, content, ec);
    if (ec) co_return false;

    fd.close();

    // Reopen the file by `::open`
    fd = co_await open_file();

    co_await uring::read(fd, ::std::as_writable_bytes(::std::span{ buffer }));
    fd.close();
    co_await delete_file();
    co_return ::std::ranges::equal(buffer, content.substr(0, 5));
}

emitter_task<bool> emit_op_fill_test()
{
    auto content = "123456789"sv;
    ::std::array<char, 5> buffer{};

    co_await delete_file();
    auto fd = co_await create_file();
    co_await uring::append_all(fd, content);

    fd.close();
    fd = co_await open_file();

    ::std::error_code ec;
    size_t readed_nbytes = co_await koios::uring::read_fill_buffer(
        10ms, fd, ::std::as_writable_bytes(::std::span{buffer}), 0, ec
    );
    co_return readed_nbytes == 5;
}

emitter_task<bool> emit_op_fill_test2()
{
    constexpr auto content = "123456789"sv;
    ::std::array<char, content.size() * 2> buffer{};

    co_await delete_file();
    auto fd = co_await create_file();
    co_await uring::append_all(fd, content);

    fd.close();
    fd = co_await open_file();

    ::std::error_code ec;
    size_t readed_nbytes = co_await koios::uring::read_fill_buffer(
        10ms, fd, ::std::as_writable_bytes(::std::span{buffer}), 0, ec
    );
    co_return readed_nbytes == content.size();
}

} // annoymous namespace

TEST(iouring, append_all)
{
    ASSERT_TRUE(append_all_test().result());
}

TEST(iouring, op_fill_buffer)
{
    ASSERT_TRUE(emit_op_fill_test().result());
    ASSERT_TRUE(emit_op_fill_test2().result());
}

#include "koios/iouring_op_batch.h"
#include "toolpex/ipaddress.h"
namespace
{
    emitter_task<bool> op_batch_test_basic()
    {
        auto ops = uring::op_batch{};
        ops.prep_socket("::1"_ip->family(), SOCK_STREAM, 0)
            .timeout(1s);
        co_await ops.execute();
        co_return ops.all_success() 
            && ops.timeout_req_ec()
            && ops.was_timeout_set();
    }
}

TEST(iouring, op_batch_basic)
{
    ASSERT_TRUE(op_batch_test_basic().result());
}
