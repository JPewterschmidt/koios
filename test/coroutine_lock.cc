#include "koios/coroutine_mutex.h"
#include "koios/coroutine_shared_mutex.h"
#include "gtest/gtest.h"
#include "koios/task.h"

#include <vector>

static size_t g_val{};
constinit size_t g_test_count{ 9999 };

// Unique

static koios::task<void> func()
{
    static koios::mutex s_mutex;
    auto lk = co_await s_mutex.acquire();
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

static koios::task<bool> try_acquire_test()
{
    koios::mutex lk;
    auto guard1 = co_await lk.acquire();
    auto guard2_opt = co_await lk.try_acquire();
    co_return !guard2_opt.has_value();
}

TEST(coro_lock, try_acquire)
{
    ASSERT_TRUE(try_acquire_test().result());
}

// Shared

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

namespace
{

koios::shared_mutex mut2;

koios::task<bool> try_get_unique_during_writing()
{
    auto shr = co_await mut2.acquire_shared();
    auto uni_opt = co_await mut2.try_acquire();
    co_return !uni_opt.has_value();
}

koios::task<bool> get_shared_during_writing()
{
    auto uni = co_await mut2.acquire();
    auto shr = co_await mut2.acquire_shared();
    co_return true;
}

} // annoymous namespace

TEST(coro_lock, shared_during_writing)
{
    ASSERT_TRUE(get_shared_during_writing().result());
    ASSERT_TRUE(try_get_unique_during_writing().result());
}
