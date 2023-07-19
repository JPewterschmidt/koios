# koios
A c++ async runtime library

## Quick Start

```c++
#include <iostream>
#include "koios/task.h"

koios::task<void> coro_task3()
{
    ::std::cout << "world!" << ::std::endl;
    co_return;
}

koios::task<int> coro_task2()
{
    ::std::cout << "koios' ";
    co_await coro_task3();
    co_return 1;
}

koios::task<int> coro_task()
{
    ::std::cout << "hello ";
    co_return co_await coro_task2();
}

int main()
{
    koios::runtime_init(12);

    int val = coro_task().run_and_get_future().get();

    return koios::runtime_exit();
}
```

