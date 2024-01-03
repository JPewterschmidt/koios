#include "koios/coroutine_mutex.h"
#include "gtest/gtest.h"
#include "koios/task.h"

#include <vector>

static size_t g_val{};
static koios::mutex g_mutex;
constinit size_t g_test_count{ 9999 };

static koios::task<void> func()
{
    auto lk = co_await g_mutex.acquire();
    ++g_val;
    lk.unlock();
    
    co_await lk.lock();
    ++g_val;
}

TEST(coro_lock, basic)
{
    ::std::vector<koios::future<void>> fvec;
    for (size_t i{}; i < g_test_count; ++i)
    {
        fvec.emplace_back(func().run_and_get_future());
    }
    for (auto& f : fvec)
        f.get();
    ASSERT_EQ(g_val, 2 * g_test_count);
}
