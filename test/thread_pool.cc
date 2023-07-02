#include "gtest/gtest.h"
#include "koios/thread_pool.h"

using namespace koios;

constinit size_t test_size{ 100000 };
constinit size_t pool_size{ 10 };

TEST(thread_pool, basic)
{
    thread_pool tp{ pool_size };
    ::std::atomic_size_t count{ test_size };

    for (size_t i = 0; i < test_size / 10; ++i)
    {
        tp.enqueue([&]{ count.fetch_sub(1u); });
        tp.enqueue([&]{ count.fetch_sub(1u); });
        tp.enqueue([&]{ count.fetch_sub(1u); });
        tp.enqueue([&]{ count.fetch_sub(1u); });
        tp.enqueue([&]{ count.fetch_sub(1u); });
        tp.enqueue([&]{ count.fetch_sub(1u); });
        tp.enqueue([&]{ count.fetch_sub(1u); });
        tp.enqueue([&]{ count.fetch_sub(1u); });
        tp.enqueue([&]{ count.fetch_sub(1u); });
        tp.enqueue([&]{ count.fetch_sub(1u); });
    }
    
    ASSERT_EQ(count.load(), 1);
    tp.stop();
    ASSERT_EQ(tp.size(), 0);
}
