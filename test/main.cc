#include "gtest/gtest.h"
#include "koios/task_scheduler_concept.h"
#include "koios/runtime.h"

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    koios::runtime_init(20);
    auto result = RUN_ALL_TESTS();
    //for (int i = 0; i < 10; ++i)
    //{
    //    result = RUN_ALL_TESTS();
    //}
    koios::runtime_exit();
    return result;
}
