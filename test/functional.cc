#include "koios/functional.h"
#include "gtest/gtest.h"
#include <print>
#include <format>
#include <ranges>

using namespace koios;
namespace r = ::std::ranges;
namespace rv = ::std::ranges::views;

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

    lazy_task<void> for_each_dummy(
        int i, 
        const ::std::vector<int>& ivec1, 
        const ::std::vector<int>& ivec2, 
        ::std::vector<int>& out)
    {
        out[i] = ivec1[i] + ivec2[i];
        co_return;
    }

    lazy_task<bool> emit_for_each_test()
    {
        auto ivec1 = rv::iota(0, 5) | r::to<::std::vector>();
        auto ivec2 = rv::iota(0, 5) | r::to<::std::vector>();
        ::std::vector<int> result(ivec1.size(), 0);

        co_await for_each(rv::iota(0, 5), for_each_dummy, ivec1, ivec2, result);

        co_return result == ::std::vector{ 0,2,4,6,8, };
    }
}

TEST(functional, co_await_all)
{
    ASSERT_TRUE(emit_co_await_all_tests().result());
    ASSERT_TRUE(emit_co_await_all_range().result());
    ASSERT_TRUE(emit_co_await_all_range_nothing().result());
}

TEST(functional, for_each)
{
    ASSERT_TRUE(emit_for_each_test().result());
}
