#include "gtest/gtest.h"
#include "koios/invocable_queue_wrapper.h"
#include "koios/std_queue_wrapper.h"

namespace
{
    int basic_count{};
}

using namespace koios;

TEST(invocable_queue_wrapper, basic)
{
    koios::invocable_queue_wrapper iqw{ std_queue_wrapper{} };
    for (size_t i{}; i < 10; ++i)
        iqw.enqueue([]{ ++basic_count; });

    for (size_t i{}; i < 10; ++i)
    {
        auto ret = iqw.dequeue();
        if (ret) (*ret)();
    }

    ASSERT_EQ(basic_count, 10);
    ASSERT_EQ(iqw.empty(), true);
}
