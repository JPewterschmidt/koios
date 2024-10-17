#include "gtest/gtest.h"
#include "koios/task_scheduler_concept.h"
#include "koios/runtime.h"

int main(int argc, char** argv)
{
    size_t thrs = 20;
    ::testing::InitGoogleTest(&argc, argv);
    koios::runtime_init(thrs);
    auto result = RUN_ALL_TESTS();
    //for (int i = 0; i < 10; ++i)
    //{
    //    result = RUN_ALL_TESTS();
    //}
    koios::runtime_exit();
    return result;
}
