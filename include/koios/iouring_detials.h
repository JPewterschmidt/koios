#ifndef KOIOS_IOURING_DETIALS_H
#define KOIOS_IOURING_DETIALS_H

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "koios/iouring_ioret.h"
#include <cstdint>
#include <cstddef>
#include <system_error>

namespace koios::uring
{
    class ioret_for_data_deliver : public ioret
    {
    public:
        ioret_for_data_deliver(ioret r) noexcept;

        size_t nbytes_delivered() const noexcept
        {
            return ret >= 0 ? static_cast<size_t>(ret) : 0;
        }

        ::std::error_code error_code() const noexcept;

    private:
        int m_errno{};
    };

    namespace detials
    {
        class iouring_aw_for_data_deliver : public iouring_aw
        {
        public:
            template<typename... Args>
            iouring_aw_for_data_deliver(Args&&... args)
                : iouring_aw(::std::forward<Args>(args)...)
            {
            }

            ioret_for_data_deliver await_resume();
        };
    }
}

#endif
