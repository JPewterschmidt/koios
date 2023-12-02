#include "gtest/gtest.h"
#include "koios/timer.h"
#include "koios/event_loop.h"
#include "koios/runtime.h"
#include "koios/task.h"
#include "koios/this_task.h"

#include <semaphore>
#include <vector>

using namespace koios;
using namespace ::std::chrono_literals;

namespace 
{
    int flag1{};
    int flag2{};
    int flag3{};

    ::std::binary_semaphore bs{0};
}

task<void> func()
{
    co_await this_task::sleep_for(5ms);
    flag1 = 1;
    co_await this_task::sleep_for(5ms);
    flag2 = 2;
}

task<void> fuckyou()
{
    flag3 = 3; 
    bs.release();
    co_return;
}

TEST(timer, basic)
{
    get_task_scheduler().add_event<timer_event_loop>(5ms, fuckyou());
    bs.acquire();
    ASSERT_EQ(flag3, 3);
}

TEST(timer, awaitable)
{
    func().result();
    ASSERT_EQ(flag1, 1);
    ASSERT_EQ(flag2, 2);
}

namespace
{
    ::std::mutex ivec_lock;
    ::std::vector<int> ivec;
}

task<void> func1()
{
    ::std::unique_lock lk{ ivec_lock };
    ivec.push_back(1);
    lk.unlock();
    co_await this_task::sleep_for(60ms);
    lk.lock();
    ivec.push_back(4);
    co_return;
}

task<void> func2()
{
    ::std::unique_lock lk{ ivec_lock };
    ivec.push_back(2);
    lk.unlock();
    co_await this_task::sleep_for(60ms);
    lk.lock();
    ivec.push_back(5);
    co_return;
}

task<void> func3()
{
    ::std::unique_lock lk{ ivec_lock };
    ivec.push_back(3);
    lk.unlock();
    co_await this_task::sleep_for(60ms);
    lk.lock();
    ivec.push_back(6);
    bs.release();
    co_return;
}

TEST(timer, several_events)
{
    get_task_scheduler().add_event<timer_event_loop>(20ms, func1());
    get_task_scheduler().add_event<timer_event_loop>(40ms, func2());
    get_task_scheduler().add_event<timer_event_loop>(60ms, func3());

    bs.acquire();
    ::std::vector correct_result{1,2,3,4,5,6};
    EXPECT_EQ(ivec, correct_result);
}
