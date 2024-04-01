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
#include "koios/from_result.h"

#include "toolpex/tic_toc.h"
#include "toolpex/errret_thrower.h"
#include "toolpex/unique_posix_fd.h"
#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include "koios/coroutine_mutex.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/invocable_atomic_queue_wrapper.h"
#include <string_view>

#include "koios/unique_file_state.h"
#include "koios/task_release_once.h"
#include "koios/iouring_op_batch.h"

#include <fcntl.h>
#include <sys/stat.h>

#include "benchmark/benchmark.h"

using namespace koios;
using namespace ::std::chrono_literals;
using namespace ::std::string_view_literals;
using namespace toolpex::ip_address_literals;

static constexpr size_t scale{ 1000000 };

static void BM_thread_pool_original(benchmark::State& state)
{
    thread_pool tp{ 2, work_stealing_queue<moodycamel_queue_wrapper>{} };
    tp.start();
    for (auto _ : state)
    {
        ::std::atomic_size_t flagv{};
        for (size_t i{}; i < scale; ++i)
            tp.enqueue_no_future([&] mutable { flagv.fetch_add(1, ::std::memory_order_relaxed); });
        while (flagv.load(::std::memory_order_relaxed) != scale);
    }
}
BENCHMARK(BM_thread_pool_original);

static void BM_thread_pool_new(benchmark::State& state)
{
    thread_pool tp{ 2, work_stealing_queue<invocable_atomic_queue_wrapper>{65536} };
    tp.start();
    for (auto _ : state)
    {
        ::std::atomic_size_t flagv{};
        for (size_t i{}; i < scale; ++i)
            tp.enqueue_no_future([&] mutable { flagv.fetch_add(1, ::std::memory_order_relaxed); });
        while (flagv.load(::std::memory_order_relaxed) != scale);
    }
}
BENCHMARK(BM_thread_pool_new);

BENCHMARK_MAIN();
