#include "koios/coroutine_mutex.h"
#include "koios/coroutine_shared_mutex.h"
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

static size_t g_val2{};

static koios::task<koios::unique_lock<koios::mutex>> func2()
{
    static koios::mutex lk;
    auto guard = co_await lk.acquire();
    g_val2++;
    co_return guard;
}

static koios::task<void> func3()
{
    auto lk = co_await func2();
    g_val2++;
}

TEST(coro_lock, move_func)
{
    g_val = g_val2 = 0;

    ::std::vector<koios::future<void>> fvec;
    for (size_t i{}; i < g_test_count; ++i)
        fvec.emplace_back(func3().run_and_get_future());
    for (auto& f : fvec)
        f.get();
    ASSERT_EQ(g_val2, 2 * g_test_count);
}

namespace
{

::std::atomic_int val;
koios::shared_mutex mut;

koios::task<bool> reader()
{
    auto lk = co_await mut.acquire_shared();
    const int old_val = val;
    for (size_t i{}; i < 1000; ++i)
    {
        if (val.load(::std::memory_order_relaxed) != old_val) 
            co_return false;
    }
    co_return true;
}

koios::task<> writer()
{
    auto lk = co_await mut.acquire();
    val.fetch_add(1, ::std::memory_order_relaxed);
}

} // namespace annoymous

TEST(coro_lock, shared)
{
    for (size_t i{}; i < 1000; ++i)
    {
        writer().run();
        ASSERT_TRUE(reader().result());
    }
}
