#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/moodycamel_queue_wrapper.h"

using namespace koios;

constinit size_t test_size{ 10000 };
constinit size_t pool_size{ 10 };

TEST(thread_pool, basic)
{
    thread_pool tp{ pool_size, moodycamel_queue_wrapper{} };
    tp.start();
    ::std::atomic_size_t count{ test_size };

    for (size_t i = 0; i < test_size / 10; ++i)
    {
        tp.enqueue_no_future([&]{ count.fetch_sub(1, ::std::memory_order_relaxed); });
        tp.enqueue_no_future([&]{ count.fetch_sub(1, ::std::memory_order_relaxed); });
        tp.enqueue_no_future([&]{ count.fetch_sub(1, ::std::memory_order_relaxed); });
        tp.enqueue_no_future([&]{ count.fetch_sub(1, ::std::memory_order_relaxed); });
        tp.enqueue_no_future([&]{ count.fetch_sub(1, ::std::memory_order_relaxed); });
        tp.enqueue_no_future([&]{ count.fetch_sub(1, ::std::memory_order_relaxed); });
        tp.enqueue_no_future([&]{ count.fetch_sub(1, ::std::memory_order_relaxed); });
        tp.enqueue_no_future([&]{ count.fetch_sub(1, ::std::memory_order_relaxed); });
        tp.enqueue_no_future([&]{ count.fetch_sub(1, ::std::memory_order_relaxed); });
        tp.enqueue_no_future([&]{ count.fetch_sub(1, ::std::memory_order_relaxed); });
    }
    
    tp.stop();
    ASSERT_EQ(count.load(), 0);
}

TEST(thread_pool, actual_threads)
{
    const size_t num = get_task_scheduler().number_of_threads();
    ASSERT_EQ(num, get_task_scheduler().consumer_attrs().size());
    ASSERT_NE(num, 0);
}
