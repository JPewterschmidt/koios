#include "fmt/core.h"
#include "gtest/gtest.h"
#include "koios/task_scheduler_concept.h"
#include "koios/global_task_scheduler.h"

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    task_scheduler_concept auto& scheduler = koios::get_task_scheduler();
    scheduler.quick_stop();
    return result;
}
