#ifndef KOIOS_IOURING_READ_AW_H
#define KOIOS_IOURING_READ_AW_H

#include <system_error>
#include <cerrno>

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_ioret.h"

#include "toolpex/unique_posix_fd.h"

KOIOS_NAMESPACE_BEG

namespace io
{
    class ioret_for_reading : public ioret
    {
    public:
        ioret_for_reading(ioret r) noexcept;

        size_t nbytes_read() const noexcept
        {
            return ret >= 0 ? static_cast<size_t>(ret) : 0;
        }

        ::std::error_code error_code() const noexcept;

    private:
        int m_errno{};
    };

    class read : public iouring_aw
    {
    public:
        read(const toolpex::unique_posix_fd& fd, 
                        ::std::span<unsigned char> buffer, 
                        uint64_t offset = 0);

        ioret_for_reading await_resume();
    };
}

KOIOS_NAMESPACE_END

#endif
