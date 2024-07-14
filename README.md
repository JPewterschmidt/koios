# koios
A c++ async runtime library

## Quick Start

```c++
#include <iostream>
#include "koios/task.h"
#include "koios/runtime.h"

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

    int val = coro_task().result();

    return koios::runtime_exit();
}
```

## Credits

- [ConcurrentQueue](https://github.com/cameron314/concurrentqueue):A fast multi-producer, multi-consumer lock-free concurrent queue for C++11.
- [fmt](https://fmt.dev):A modern formatting library
- [Magic Enum](https://github.com/Neargye/magic_enum): Static reflection for enums (to string, from string, iteration) for modern C++, work with any enum type without any macro or boilerplate code.
- [gtest](https://google.github.io/googletest/): GoogleTest - Google Testing and Mocking Framework.
- [gflags](https://gflags.github.io/gflags/):The gflags package contains a C++ library that implements commandline flags processing. It includes built-in support for standard types such as string and the ability to define flags in the source file in which they are used.
- [benchmark](https://github.com/google/benchmark): A microbenchmark support library.
- [xmake](https://xmake.io): 🔥 A cross-platform build utility based on Lua 
- [toolpex](https://github.com/JPewterschmidt/toolpex): A C++ Utility Toolbox.

