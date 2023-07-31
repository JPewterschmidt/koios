#include <liburing.h>
#include <sys/mman.h>

#include "koios/uring_detial/io_uring_eventloop.h"
#include "koios/exceptions.h"
#include "toolpex/errret_thrower.h"

using namespace koios::uring::detial;

io_uring_eventloop::io_uring_eventloop(size_t num_of_entries)
{
    toolpex::errret_thrower<koios::uring_exception> et;
    et << ::io_uring_queue_init(num_of_entries, &m_ring, 0);
}

io_uring_eventloop::~io_uring_eventloop() noexcept
{
    ::io_uring_queue_exit(&m_ring);
}

