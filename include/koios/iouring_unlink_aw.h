#ifndef KOIOS_IOURING_UNLINK_AW_H
#define KOIOS_IOURING_UNLINK_AW_H

#include <string_view>
#include <filesystem>

#include "koios/macros.h"
#include "koios/iouring_aw.h"

namespace koios::uring
{
    class unlink : public iouring_aw
    {
    public:
        unlink(::std::string_view path, int flags = 0);
        unlink(::std::filesystem::path path, int flags = 0);
    };
}

#endif
