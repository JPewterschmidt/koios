#ifndef KOIOS_UTILITY_H
#define KOIOS_UTILITY_H

#include "koios/macros.h"
#include "koios/task.h"

#include <concepts>

KOIOS_NAMESPACE_BEG

auto identity(auto arg) -> task<decltype(arg)>
{
    co_return arg;
}

KOIOS_NAMESPACE_END

#endif
