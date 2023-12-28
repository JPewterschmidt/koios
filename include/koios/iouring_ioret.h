#ifndef KOIOS_IOURING_IORET_H
#define KOIOS_IOURING_IORET_H

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

struct ioret
{
    int32_t ret{};
    uint32_t flags{};
};

KOIOS_NAMESPACE_END

#endif
