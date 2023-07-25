#include <liburing.h>
#include <sys/mman.h>

#include "koios/uring_detial/uring_queue.h"
#include "koios/exceptions.h"

using namespace koios::uring::detial;

uring_storage::uring_storage(int ring_fd, const ::io_uring_params* const p)
    : m_sq_size{ p->sq_entries }, m_cq_size{ p->cq_entries }
{
    size_t sring_nbytes = p->sq_off.array + p->sq_entries * sizeof(unsigned);
    size_t cring_nbytes = p->cq_off.cqes  + p->cq_entries * sizeof(::io_uring_cqe);

    if (p->features & IORING_FEAT_SINGLE_MMAP)
    {
        if (cring_nbytes > sring_nbytes)
            sring_nbytes = cring_nbytes;
        cring_nbytes = sring_nbytes;
    }

    void* sq_ptr = ::mmap(0, sring_nbytes, PROT_READ | PROT_WRITE, 
                          MAP_SHARED | MAP_POPULATE, 
                          ring_fd, IORING_OFF_SQ_RING);
    if (sq_ptr == MAP_FAILED)
    {
        throw koios::uring_exception{ "mmap for sring" };
    }
    m_sq_ptr = static_cast<unsigned*>(sq_ptr);
    m_sq_nbytes = sring_nbytes;

    void* cq_ptr{};
    if (p->features & IORING_FEAT_SINGLE_MMAP)
    {
        cq_ptr = sq_ptr;
    }
    else
    {
        cq_ptr = ::mmap(0, cring_nbytes, PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_POPULATE, 
                        ring_fd, IORING_OFF_CQ_RING);
        if (cq_ptr == MAP_FAILED)
        {
            ::munmap(sq_ptr, sring_nbytes);
            throw koios::uring_exception{ "mmap for cring" };
        }
    }
    m_cq_ptr = static_cast<::io_uring_cqe*>(cq_ptr);
    m_cq_nbytes = cring_nbytes;
    
    const size_t sqes_nb = p->sq_entries * sizeof(::io_uring_sqe);
    void* sqes = ::mmap(0, sqes_nb, 
                  PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, 
                  ring_fd, IORING_OFF_SQES);
    if (sqes == MAP_FAILED)
    {
        ::munmap(sq_ptr, sring_nbytes);
        if (sq_ptr != cq_ptr)
            ::munmap(cq_ptr, cring_nbytes);

        throw koios::uring_exception{ "mmap for sqe" };
    }
    m_sqes = static_cast<::io_uring_sqe*>(sqes);
    m_sqes_nbytes = sqes_nb;
}

uring_storage::~uring_storage() noexcept
{
    // default initialized
    if (m_sq_ptr == nullptr)
        return;

    ::munmap(m_cq_ptr, m_cq_nbytes);
    if ((void*)m_cq_ptr != (void*)m_sq_ptr)
        ::munmap(m_sq_ptr, m_sq_nbytes);
    ::munmap(m_sqes, m_sqes_nbytes);
}

cq uring_storage::get_cq() noexcept
{
    return { m_cq_ptr, m_cq_size };
}

sq uring_storage::get_sq() noexcept
{
    return { m_sq_ptr, m_sq_size, m_sqes };
}
