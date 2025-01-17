#include "koios/functional.h"
#include "gtest/gtest.h"

using namespace koios;

namespace
{
    task<int> co_await_all1()
    {
        co_return 1;
    }

    task<double> co_await_all2()
    {
        co_return 1.0;
    }

    lazy_task<bool> emit_co_await_all_tests()
    {
        auto [i, d] = co_await co_await_all(co_await_all1(), co_await_all2());
        co_return i == 1 && (d - 1) < 10e-3;
    }
    
    lazy_task<> emit_nothing()
    {
        co_return;
    }

    lazy_task<bool> emit_co_await_all_range()
    {
        ::std::vector<koios::future<int>> ifuts {};
        ifuts.push_back(co_await_all1().run_and_get_future());
        ifuts.push_back(co_await_all1().run_and_get_future());
        ifuts.push_back(co_await_all1().run_and_get_future());
        ifuts.push_back(co_await_all1().run_and_get_future());
        ifuts.push_back(co_await_all1().run_and_get_future());
        ifuts.push_back(co_await_all1().run_and_get_future());

        const size_t ifuts_size = ifuts.size();
        auto ret = co_await co_await_all(::std::move(ifuts));
        co_return ret.size() == ifuts_size && ret[0] == ret[1];
    }

    lazy_task<bool> emit_co_await_all_range_nothing()
    {
        ::std::vector<koios::future<void>> ifuts {};
        ifuts.push_back(emit_nothing().run_and_get_future());
        ifuts.push_back(emit_nothing().run_and_get_future());
        ifuts.push_back(emit_nothing().run_and_get_future());
        ifuts.push_back(emit_nothing().run_and_get_future());
        ifuts.push_back(emit_nothing().run_and_get_future());
        ifuts.push_back(emit_nothing().run_and_get_future());

        co_await co_await_all(::std::move(ifuts));

        co_return true;
    }
}

TEST(task, co_await_all)
{
    ASSERT_TRUE(emit_co_await_all_tests().result());
    ASSERT_TRUE(emit_co_await_all_range().result());
    ASSERT_TRUE(emit_co_await_all_range_nothing().result());
}

