#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/moodycamel_queue_wrapper.h"

using namespace koios;

constinit size_t test_size{ 100000 };
constinit size_t pool_size{ 10 };

TEST(thread_pool, basic)
{
    thread_pool tp{ pool_size, moodycamel_queue_wrapper{} };
    ::std::atomic_size_t count{ test_size };

    for (size_t i = 0; i < test_size / 10; ++i)
    {
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
        tp.enqueue([&]{ --count; });
    }
    
    tp.stop();
    ASSERT_EQ(count.load(), 0);
}

TEST(thread_pool, after_quick_stop)
{
    thread_pool tp{ pool_size, moodycamel_queue_wrapper{} };
    tp.quick_stop();
    int success{};
    try
    {
        tp.enqueue([]{});
    }
    catch (koios::thread_pool_stopped_exception& [[maybe_unused]] e)
    {
        success = 1;
    }
    ASSERT_EQ(success, 1);
}
