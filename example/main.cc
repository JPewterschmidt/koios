#include <vector>

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "koios/task.h"

using namespace koios;

int main()
{
    ::std::vector ivec{ 1,2,3,4,5 };
    fmt::print("{}\n", ivec);
}
