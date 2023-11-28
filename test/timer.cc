#include "gtest/gtest.h"
#include "koios/timer.h"
#include "koios/event_loop.h"
#include "koios/runtime.h"
#include "koios/task.h"
#include "koios/this_task.h"

#include <semaphore>

using namespace koios;
using namespace ::std::chrono_literals;

int flag1{};
int flag2{};
int flag3{};

::std::binary_semaphore bs{0};

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
