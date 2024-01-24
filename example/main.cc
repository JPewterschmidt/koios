#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"
#include <chrono>
#include <iostream>
#include <cassert>
#include <fcntl.h>
#include <atomic>
#include <vector>
#include <fstream>

#include "koios/work_stealing_queue.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/this_task.h"
#include "koios/expected.h"
#include "koios/functional.h"

#include "toolpex/tic_toc.h"
#include "toolpex/errret_thrower.h"
#include "toolpex/unique_posix_fd.h"
#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include "koios/coroutine_mutex.h"
#include <string_view>
#include "koios/iouring_connect_aw.h"

#include "koios/unique_file_state.h"

#include <fcntl.h>
#include <sys/stat.h>

using namespace koios;
using namespace ::std::chrono_literals;
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
    auto fd = co_await create_file();
    
    co_await uring::append_all(fd, content);

    ::std::error_code ec;
    co_await uring::append_all(fd, content, ec);
    if (ec) co_return false;

    fd.close();
    fd = co_await open_file();

    co_await uring::read(fd, ::std::as_writable_bytes(::std::span{ buffer }));
    fd.close();
    co_await delete_file();
    co_return ::std::ranges::equal(buffer, content.substr(0, 5));
}

} // annoymous namespace

int main()
try
{
    runtime_init(4);

    ::std::cout << append_all_test().result() << ::std::endl;

    runtime_exit();
    
    return 0;
}
catch (const ::std::exception& e)
{
    ::std::cout << e.what() << ::std::endl;
    koios::runtime_exit();
}
