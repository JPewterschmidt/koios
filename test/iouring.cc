#include "gtest/gtest.h"
#include "koios/iouring_awaitables.h"
#include "toolpex/errret_thrower.h"

#include <ranges>
#include <string_view>
#include <memory>

using namespace koios;
using namespace ::std::string_view_literals;

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

task<bool> append_all_test()
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

} // annoymous namespace

TEST(iouring, append_all)
{
    ASSERT_TRUE(append_all_test().result());
}
