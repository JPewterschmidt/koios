#include "gtest/gtest.h"
#include "koios/timer.h"
#include "koios/event_loop.h"
#include "koios/runtime.h"
#include "koios/task.h"
#include "koios/this_task.h"
#include "koios/functional.h"

#include <semaphore>
#include <vector>
#include <chrono>
#include <mutex>

using namespace koios;
using namespace ::std::chrono_literals;

namespace 
{
int flag1{};
int flag2{};

::std::binary_semaphore bs{0};

lazy_task<void> func()
{
    co_await this_task::sleep_for(5ms);
    flag1 = 1;
    co_await this_task::sleep_for(5ms);
    flag2 = 2;
}

TEST(timer, awaitable)
{
    func().result();
    ASSERT_EQ(flag1, 1);
    ASSERT_EQ(flag2, 2);
}

::std::mutex ivec_lock;
::std::vector<int> ivec;

task<> func1()
{
    ::std::unique_lock lk{ ivec_lock };
    ivec.push_back(1);
    lk.unlock();
    co_await this_task::sleep_for(8ms);
    lk.lock();
    ivec.push_back(4);
    co_return;
}

task<> func2()
{
    ::std::unique_lock lk{ ivec_lock };
    ivec.push_back(2);
    lk.unlock();
    co_await this_task::sleep_for(8ms);
    lk.lock();
    ivec.push_back(5);
    co_return;
}

task<> func3()
{
    ::std::unique_lock lk{ ivec_lock };
    ivec.push_back(3);
    lk.unlock();
    co_await this_task::sleep_for(8ms);
    lk.lock();
    ivec.push_back(6);
    bs.release();
    co_return;
}

lazy_task<void> mainfunc()
{
    // add_event should never be called in main thread.
    get_task_scheduler().add_event<timer_event_loop>(2ms, make_lazy(func1));
    get_task_scheduler().add_event<timer_event_loop>(4ms, make_lazy(func2));
    get_task_scheduler().add_event<timer_event_loop>(8ms, make_lazy(func3));
    co_return;
}

lazy_task<bool> test_sleep_until_3ms()
{
    auto now = ::std::chrono::system_clock::now();
    co_await this_task::sleep_until(now + 3ms);
    co_return ::std::chrono::duration_cast<::std::chrono::milliseconds>(
        ::std::chrono::system_clock::now() - now) == 3ms;
}

}

TEST(timer, several_events)
{
    flag1 = flag2 = false;
    ivec = ::std::vector<int>{};

    mainfunc().run();
    bs.acquire();
    ::std::vector correct_result{1,2,3,4,5,6};
    EXPECT_EQ(ivec, correct_result);
}

TEST(timer, sleep_until_3ms_later)
{
    flag1 = flag2 = false;
    ivec = ::std::vector<int>{};

    ASSERT_TRUE(test_sleep_until_3ms().result());
}
