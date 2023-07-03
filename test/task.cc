#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/task.h"

using namespace koios;

int flag{};

task<void> for_basic_test()
{
    flag = 1;
    co_return;
}

TEST(task, basic)
{
    auto t = for_basic_test();
    t();
    ASSERT_EQ(flag, 1);
}

