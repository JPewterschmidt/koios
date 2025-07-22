#include "gtest/gtest.h"
#include "koios/task.h"
#include "koios/lite_task.h"
#include "koios/iouring_awaitables.h"

#include <ranges>

using namespace koios;
namespace r = ::std::ranges;
namespace rv = ::std::views;

namespace
{
    // ================== lifetime test ===================
    bool hascopied{};
    class lifetime
    {
    public:
        lifetime() = default;
        lifetime(const lifetime&) 
        { 
            hascopied = true; 
        }
        lifetime& operator= (const lifetime&) 
        { 
            hascopied = true; return *this; 
        }
        lifetime(lifetime&&) noexcept = default;
        lifetime& operator=(lifetime&&) noexcept = default;
    };
    
    lite_task<lifetime> lifetime_lt()
    {
        lifetime ret;
        co_return ret;
    }

    task<bool> should_not_copy()
    {
        [[maybe_unused]] auto ret = co_await lifetime_lt();
        co_return !hascopied;
    }

    // ================== io test ===================
    
    lite_task<bool> io_test()
    {
        co_await uring::nop();
    }

    lazy_task<> io_emitter()
    {
        co_await io_test();
    }

} // annoymouse namespace

TEST(lite_task, lifetime)
{
    ASSERT_TRUE(should_not_copy().result());
    hascopied = false;
}

TEST(lite_task, iouring_compatible)
{
    io_emitter().result();
}
