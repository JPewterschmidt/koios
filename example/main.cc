#include <vector>

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "koios/task.h"
#include "koios/thread_pool.h"
#include "koios/from_result.h"
#include "koios/generator.h"

#include "toolpex/unique_resource.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <future>

koios::generator<int> g(int last_val)
{
    for (int i = 1; i <= last_val; ++i)
        co_yield i;
}

int main()
{
    auto gg = g(10);
    int val{};

    for (auto i = gg.begin(); i != gg.end(); ++i)
    {
        val = *i;
        val = *i;
        ::std::cout << val << ::std::endl;
    }

    return 0;
}
