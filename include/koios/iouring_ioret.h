#ifndef KOIOS_IOURING_IORET_H
#define KOIOS_IOURING_IORET_H

#include <cstdint>
#include <system_error>

namespace koios::uring
{
    struct ioret
    {
        int32_t ret{};
        uint32_t flags{};
    };

    namespace detials
    {
        class ioret_for_any_base : public ioret
        {
        public:
            ioret_for_any_base(ioret r) noexcept;
            ::std::error_code error_code() const noexcept;

        private:
            int m_errno{};
        };
    }
}

#endif
