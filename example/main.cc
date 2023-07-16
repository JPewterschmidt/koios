#include <vector>
#include <functional>

#include "fmt/core.h"
#include "fmt/ranges.h"
#include "glog/logging.h"

#include "koios/task.h"
#include "koios/monad_task.h"
#include "koios/thread_pool.h"
#include "koios/from_result.h"
#include "koios/generator.h"

#include "toolpex/unique_resource.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <future>
#include <ranges>
#include <iterator>

using namespace koios;

struct foo
{
    foo() = default;
    foo(const foo&) { ::std::cout << "copy\n"; }
    foo(foo&&) { ::std::cout << "move\n"; }
};

::std::vector<foo> fvec(10);
koios::generator<foo> g1()
{
    for (auto& f : fvec)
        co_yield ::std::move(f);
}

int main()
{
    auto g = g1();
    for (const auto& f : g)
    {
        ::std::cout << "ok" << ::std::endl;
    }

}
