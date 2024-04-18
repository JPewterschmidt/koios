#include "gtest/gtest.h"
#include "koios/dir_mutex.h"
#include "koios/this_task.h"
#include "koios/iouring_awaitables.h"
#include <filesystem>

#include <atomic>
#include <chrono>
#include <vector>

namespace 
{

namespace fs = ::std::filesystem;
using namespace koios;
using namespace ::std::chrono_literals;

class dir_mutex_test : public ::testing::Test
{
public:
    dir_mutex_test()
        : m_lock{ fs::current_path() }
    {
        clean().result();
    }

    eager_task<bool> acquire_test()
    {
        try
        {
            [[maybe_unused]] auto guard = co_await m_lock.acquire();
        }
        catch (...)
        {
            co_return false;
        }
        co_return true;
    }

    eager_task<bool> execlusive_availability()
    {
        auto guard = co_await m_lock.acquire();
        co_await this_task::sleep_for(50ms);
        bool expected = false;
        bool result = m_holded.compare_exchange_strong(expected, true);
        m_holded.store(false);
        co_return result;
    }

    eager_task<> clean()
    {
        co_await uring::unlink("koios_dir_lock");
    }

    ~dir_mutex_test() noexcept
    {
        clean().result();
        for (auto& item : m_futs)
        {
            (void)item.get();
        }
    }

    ::std::vector<koios::future<bool>> m_futs;

private:
    dir_mutex m_lock;
    ::std::atomic_bool m_holded;
};

} // annoymous namespace

TEST_F(dir_mutex_test, acquire)
{
    ASSERT_TRUE(acquire_test().result());
}

TEST_F(dir_mutex_test, multi_lock)
{
    m_futs.emplace_back(execlusive_availability().run_and_get_future());
    ASSERT_TRUE(execlusive_availability().result());
    m_futs.emplace_back(execlusive_availability().run_and_get_future());
    ASSERT_TRUE(execlusive_availability().result());

    for (auto& item : m_futs)
    {
        (void)item.get();
    }
}
