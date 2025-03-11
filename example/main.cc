#include "toolpex/assert.h"
#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"
#include <chrono>
#include <fcntl.h>
#include <atomic>
#include <vector>
#include <fstream>
#include <print>

#include "koios/work_stealing_queue.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/this_task.h"
#include "koios/expected.h"
#include "koios/functional.h"
#include "koios/from_result.h"
#include "koios/generator.h"

#include "toolpex/tic_toc.h"
#include "toolpex/errret_thrower.h"
#include "toolpex/unique_posix_fd.h"

#include "koios/wait_group.h"
#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include "koios/coroutine_mutex.h"
#include <string_view>

#include "koios/unique_file_state.h"
#include "koios/task_release_once.h"
#include "koios/iouring_op_batch.h"

#include <fcntl.h>
#include <sys/stat.h>

using namespace koios;
using namespace ::std::chrono_literals;
using namespace ::std::string_view_literals;
using namespace toolpex::ip_address_literals;

namespace r = ::std::ranges;
namespace rv = ::std::views;

namespace
{
    lazy_task<> func1(auto tk)
    {
        co_return;
    }

    lazy_task<> main_body()
    {
        koios::wait_group wg;
        //auto futs = rv::iota(0, 10000000)
        //    | rv::transform([&](auto&& i) {
        //          return func1(wait_group_guard{wg}).run_and_get_future();
        //      })
        //    | r::to<::std::vector>()
        //    ;

        for (size_t i{}; i < 1000000; ++i)
        {
            func1(wait_group_guard{wg}).run();
        }

        co_await wg.wait();
        ::std::println("done1");
        //co_await co_await_all(::std::move(futs));
        //::std::println("done2");
        co_return;
    }
}

int main()
try
{
    koios::runtime_init(12);
    main_body().result();
    koios::runtime_exit();
    return 0;
}
catch (const ::std::exception& e)
{
    ::std::println("{}", e.what());
    koios::runtime_exit();
}
