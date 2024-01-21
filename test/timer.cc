#include "gtest/gtest.h"
#include "koios/timer.h"
#include "koios/event_loop.h"
#include "koios/runtime.h"
#include "koios/task.h"
#include "koios/this_task.h"
#include "koios/functional.h"

#include <semaphore>
#include <vector>

using namespace koios;
using namespace ::std::chrono_literals;

namespace 
{
int flag1{};
int flag2{};

::std::binary_semaphore bs{0};

emitter_task<void> func()
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
    co_await this_task::sleep_for(60ms);
    lk.lock();
    ivec.push_back(4);
    co_return;
}

task<> func2()
{
    ::std::unique_lock lk{ ivec_lock };
    ivec.push_back(2);
    lk.unlock();
    co_await this_task::sleep_for(60ms);
    lk.lock();
    ivec.push_back(5);
    co_return;
}

task<> func3()
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

emitter_task<void> mainfunc()
{
    // add_event should never be called in main thread.
    get_task_scheduler().add_event<timer_event_loop>(20ms, make_emitter(func1));
    get_task_scheduler().add_event<timer_event_loop>(40ms, make_emitter(func2));
    get_task_scheduler().add_event<timer_event_loop>(60ms, make_emitter(func3));
    co_return;
}

}

TEST(timer, several_events)
{
    mainfunc().run();
    bs.acquire();
    ::std::vector correct_result{1,2,3,4,5,6};
    EXPECT_EQ(ivec, correct_result);
}
