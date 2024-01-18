#ifndef KOIOS_UTILITY_H
#define KOIOS_UTILITY_H

#include "koios/macros.h"
#include "koios/functional.h"

KOIOS_NAMESPACE_BEG

inline auto from_result(auto&& r)
{
    return identity(::std::forward<decltype(r)>(r));
}

KOIOS_NAMESPACE_END

#endif
