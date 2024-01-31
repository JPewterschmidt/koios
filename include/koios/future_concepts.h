#ifndef KOIOS_FUTURE_CONCEPTS_H
#define KOIOS_FUTURE_CONCEPTS_H

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

template<typename Future>
concept future_concept = requires (Future f)
{
    { f.get() } -> ::std::same_as<typename Future::value_type>;
    { f.get_nonblk() } -> ::std::same_as<typename Future::value_type>;
    { f.ready() } -> ::std::same_as<bool>;
};

KOIOS_NAMESPACE_END

#endif
