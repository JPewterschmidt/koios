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

int main()
{
    ::std::cout << "ok" << ::std::endl;   
}
