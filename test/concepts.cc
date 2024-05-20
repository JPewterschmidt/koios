#include "gtest/gtest.h"

#include "toolpex/assert.h"

#include "koios/task.h"
#include "koios/task_concepts.h"

#include <coroutine>
#include <type_traits>
#include <concepts>

using namespace koios;

namespace
{
    struct dummy_aw
    {
        bool await_ready() { return true; }
        void await_resume() {}
        void await_suspend(::std::coroutine_handle<>) {}
    };

    eager_task<void> dummy_task() { co_return; }
    task<void> dummy_task2() { co_return; }
}

TEST(task_concepts, awaitable_related)
{
    ASSERT_TRUE((awaitible_concept<dummy_aw>));
    ASSERT_TRUE((awaitible_concept<::std::suspend_always>));
    ASSERT_TRUE((awaitible_concept<::std::suspend_always>));
    ASSERT_TRUE((::std::same_as<awaitable_result_type_t<dummy_aw>, void>));
    ASSERT_TRUE((is_all_same_result_aws_v<
        toolpex::get_return_type_t<decltype(dummy_task)>, 
        toolpex::get_return_type_t<decltype(dummy_task2)>
    >));
    ASSERT_TRUE((is_all_same_result_aws_v<
        toolpex::get_return_type_t<decltype(dummy_task2)>, 
        dummy_aw
    >));
}

TEST(task_concepts, task_related)
{
    ASSERT_TRUE((awaitable_callable_concept<decltype(dummy_task)>));
    ASSERT_TRUE((eager_task_callable_concept<decltype(dummy_task)>));
    ASSERT_TRUE((task_callable_concept<decltype(dummy_task)>));
}

TEST(other_traits, first_type)
{
    ASSERT_TRUE((::std::same_as<first_type_t<int, double, float>, int>));
    ASSERT_TRUE((::std::same_as<first_type_t<int, float>, int>));
    ASSERT_TRUE((::std::same_as<first_type_t<int, void>, int>));
    ASSERT_TRUE((::std::same_as<first_type_t<int>, int>));
}

TEST(other_traits, is_all_same_type)
{
    ASSERT_TRUE((is_all_same_type_v<int, int, int>));
    ASSERT_TRUE((is_all_same_type_v<int>));
    ASSERT_TRUE((is_all_same_type_v<void>));
    ASSERT_FALSE((is_all_same_type_v<void, int>));
    ASSERT_FALSE((is_all_same_type_v<>));
}

namespace
{

task<int> dummy_func()
{
    // This coroutine should not be executed 
    // since it's just a declval dummy.
    toolpex_assert(false);
    co_return {};
}

} // annoymous namespace

TEST(task_concepts, task_callable_with_result)
{
    ASSERT_TRUE((task_callable_with_result_concept<decltype(dummy_func), int>));
}
