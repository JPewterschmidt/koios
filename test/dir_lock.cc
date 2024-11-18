#include "gtest/gtest.h"

#include "koios/dir_mutex.h"
#include "koios/this_task.h"
#include "koios/iouring_awaitables.h"

#include <filesystem>
#include <atomic>
#include <chrono>
#include <vector>
#include <mutex>

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

    lazy_task<bool> acquire_test()
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

    lazy_task<bool> execlusive_availability()
    {
        auto guard = co_await m_lock.acquire();
        co_await this_task::sleep_for(50ms);
        bool expected = false;
        bool result = m_holded.compare_exchange_strong(expected, true);
        if (result) m_holded.store(false);
        else m_fail.store(false);
        co_return result;
    }

    bool execlusive_availability_test_fail() const { return m_fail.load(); }

    lazy_task<> clean()
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
    ::std::mutex m_futs_lock;

private:
    dir_mutex m_lock;
    ::std::atomic_bool m_holded;
    ::std::atomic_bool m_fail{};
};

} // annoymous namespace

TEST_F(dir_mutex_test, acquire)
{
    ASSERT_TRUE(acquire_test().result());
}

TEST_F(dir_mutex_test, multi_lock)
{
    ::std::lock_guard lk{ m_futs_lock };

    m_futs.emplace_back(execlusive_availability().run_and_get_future());
    m_futs.emplace_back(execlusive_availability().run_and_get_future());
    ASSERT_TRUE(execlusive_availability().result());
    m_futs.emplace_back(execlusive_availability().run_and_get_future());
    m_futs.emplace_back(execlusive_availability().run_and_get_future());
    ASSERT_TRUE(execlusive_availability().result());

    for (auto& item : m_futs)
    {
        (void)item.get();
    }
    m_futs.clear();

    ASSERT_FALSE(execlusive_availability_test_fail());
}
