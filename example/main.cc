#include <vector>
#include <functional>

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "koios/task.h"
#include "koios/monad_task.h"
#include "koios/thread_pool.h"
#include "koios/from_result.h"
#include "koios/generator.h"
#include "koios/invocable_queue_wrapper.h"

#include "toolpex/unique_resource.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <future>
#include <ranges>
#include <iterator>

using namespace ::std::chrono_literals;
using namespace koios;

task<void> func2(int i)
{
    if (i-- == 0) co_return;
    co_await func2(i);
}

task<void> func()
{
    co_await func2(10);
}

int main()
{
    koios::runtime_init(1);

    func().run();
    func().run();
    func().run();
    func().run();
    func().run();
    func().run();
    func().run();
    func().run();
    func().run();
    func().run();
    func().run();
    func().run();
    func().run();
    func().run();
    func().run();
    func().run();

    get_task_scheduler().stop();
    ::std::cout << get_task_scheduler().number_remain_tasks() << ::std::endl;

    return runtime_exit();
}
