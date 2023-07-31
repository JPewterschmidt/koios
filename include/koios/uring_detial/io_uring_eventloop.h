#ifndef KOIOS_IO_URING_EVENTLOOP_H
#define KOIOS_IO_URING_EVENTLOOP_H

#include <utility>
#include <cstddef>

#include <linux/io_uring.h>
#include <liburing.h>

namespace koios::uring::detial
{
    class io_uring_eventloop
    {
    public:
        constexpr io_uring_eventloop() = default;
        io_uring_eventloop(size_t num_of_entries);
        ~io_uring_eventloop() noexcept;

    private:
        struct ::io_uring m_ring{};
    };
}

#endif
