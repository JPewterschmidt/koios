#ifndef KOIOS_IOURING_IORET_H
#define KOIOS_IOURING_IORET_H

#include <cstdint>

namespace koios::uring
{
    struct ioret
    {
        int32_t ret{};
        uint32_t flags{};
    };
}

#endif
