#include "gtest/gtest.h"
#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"

#include <vector>
#include <chrono>

using namespace koios;

::std::atomic_int result{};

task<int> coro()
{
    ::std::vector tvec{ 
        +[] -> task<int> { co_return 1; }, 
        +[] -> task<int> { co_return 1; }, 
        +[] -> task<int> { co_return 1; }, 
        +[] -> task<int> { co_return 1; }, 
        +[] -> task<int> { co_return 1; }, 
    };

    int result{};

    for (const auto& i : tvec)
    {
        result += co_await i();
    }

    co_return result;
}

task<void> starter()
{
    result += co_await coro();
}

TEST(task_scheduler, basic)
{
    constexpr size_t loop_size{ 100 };
    ::std::vector<koios::future<void>> fvec{};
    for (size_t i{}; i < loop_size; ++i)
        fvec.emplace_back(starter().run_and_get_future());

    for (auto& item : fvec)
        item.get();

    ASSERT_EQ(result, 5 * loop_size);
}

namespace
{
bool success1{};
bool success2{};

emitter_task<> dummy()
{
    co_return;
}

class loop_for_test : public user_event_loop_interface
{
public:
    void thread_specific_preparation(const per_consumer_attr& attr) noexcept override 
    { 
        success1 = true;
    }
    void stop() noexcept override { }
    void quick_stop() noexcept override { }
    void until_done() override { }
    ::std::chrono::milliseconds max_sleep_duration(const per_consumer_attr& attr) noexcept override 
    { 
        using namespace ::std::chrono_literals;
        return 1000ms;
    }

    void do_occured_nonblk() noexcept override
	{
        success2 = true;
	}
};

} // annoymous namespace

TEST(event_loop, user_event_loop)
{
    success1 = success2 = false;
    result.store(0);
    get_task_scheduler().as_loop<user_event_loops>().add_loop(::std::make_shared<loop_for_test>());
    dummy().result();
    ASSERT_TRUE(success1 && success2);
}
