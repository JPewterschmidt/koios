#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/delayed_scheduler.h"
#include "koios/task_scheduler_concept.h"
#include "koios/delayed_scheduler.h"
#include "koios/task.h"
#include <chrono>
#include <iostream>

using namespace koios;
using namespace ::std::chrono_literals;

namespace
{
    task<::std::chrono::high_resolution_clock::time_point> func()
    {
        co_return ::std::chrono::high_resolution_clock::now();
    }
}

TEST(delayed_scheduler, basic)
{
    delayed_scheduler ds{ 50ms };
    const auto now = ::std::chrono::high_resolution_clock::now();
    const auto ret_tp = func().result_on(ds);
    EXPECT_TRUE(ret_tp - now >= 50ms);   
}

