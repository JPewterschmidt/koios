#include <vector>
#include <functional>

#include "fmt/core.h"
#include "fmt/ranges.h"
#include "glog/logging.h"

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

using namespace koios;

task<void> func2()
{
    ::std::cout << "func2" << ::std::endl;
    co_await func2();
}

task<void> func()
{
    ::std::cout << "func" << ::std::endl;
    co_await func2();
}

int main()
{
    func().run();
    auto& ts = get_task_scheduler();
    ts.quick_stop();
}
