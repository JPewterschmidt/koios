#include "gtest/gtest.h"

#include "koios/wait_group.h"
#include "koios/task.h"
#include "koios/this_task.h"

using namespace koios;
using namespace ::std::chrono_literals;

namespace
{
    bool flag[5]{};
    wait_group wg;

    task<> being_wait()
    {
        for (size_t i{}; i < 5; ++i)
        {
            wait_group_guard g{ wg };
            co_await this_task::sleep_for(2ms);
        }
    }

    task<> waiting(size_t i)
    {
        co_await wg.wait();
        flag[i] = true;
    }

    lazy_task<> basic_body()
    {
        being_wait().run();
        for (size_t i{}; i < 5; ++i)
        {
            co_await waiting(i);
        }
    }
}

TEST(wait_group, basic)
{
    basic_body().result();

    ASSERT_TRUE(flag[0]);
    ASSERT_TRUE(flag[1]);
    ASSERT_TRUE(flag[2]);
    ASSERT_TRUE(flag[3]);
    ASSERT_TRUE(flag[4]);
}
