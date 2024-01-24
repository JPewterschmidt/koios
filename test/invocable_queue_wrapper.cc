#include "gtest/gtest.h"
#include "koios/invocable_queue_wrapper.h"
#include "koios/std_queue_wrapper.h"
#include "koios/per_consumer_attr.h"
#include "toolpex/unique_resource.h"

namespace
{
    int basic_count{};
    int dtor_count{};
}

using namespace koios;

TEST(invocable_queue_wrapper, special_member_func)
{
    dtor_count = 0;
    {
        koios::invocable_queue_wrapper iqw{ std_queue_wrapper{} };

        for (size_t i{}; i < 10; ++i)
            iqw.enqueue([h = toolpex::unique_resource(1, [](int*){ ++dtor_count; })]{ ++basic_count; });

        koios::invocable_queue_wrapper iqw2{ ::std::move(iqw) };

        for (size_t i{}; i < 5; ++i)
        {
            auto ret = iqw2.dequeue(per_consumer_attr{});
            if (ret) (*ret)();
        }
    }
    ASSERT_EQ(dtor_count, 10);
}

TEST(invocable_queue_wrapper, basic)
{
    basic_count = 0;
    koios::invocable_queue_wrapper iqw{ std_queue_wrapper{} };
    for (size_t i{}; i < 10; ++i)
        iqw.enqueue([]{ ++basic_count; });

    for (size_t i{}; i < 10; ++i)
    {
        auto ret = iqw.dequeue(per_consumer_attr{});
        if (ret) (*ret)();
    }

    ASSERT_EQ(basic_count, 10);
    ASSERT_EQ(iqw.empty(), true);
}
