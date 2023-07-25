#ifndef KOIOS_URING_URING_QUEUE_H
#define KOIOS_URING_URING_QUEUE_H

#include <utility>
#include <cstddef>

#include <linux/io_uring.h>

namespace koios::uring::detial
{
    class cq;
    class sq;

    class uring_storage
    {
    public:
        constexpr uring_storage() = default;
        uring_storage(int ring_fd, const ::io_uring_params* const p);
        ~uring_storage() noexcept;

        cq get_cq() noexcept;
        sq get_sq() noexcept;

    private:
        size_t m_sq_size{}, m_cq_size{};

        unsigned*       m_sq_ptr; size_t m_sq_nbytes{}; 
        ::io_uring_cqe* m_cq_ptr; size_t m_cq_nbytes{}; 
        ::io_uring_sqe* m_sqes;   size_t m_sqes_nbytes{}; 
    };

    class ring_queue
    {
    protected:
        ring_queue(void* ring_buffer, size_t size) noexcept
            : m_q_buffer{ ring_buffer }, m_size{ size }
        {
        }

        template<typename T>
        auto& raw_queue() noexcept
        {
            return *static_cast<T*>(m_q_buffer);
        }
        
    public: 
        size_t size() const noexcept { return m_size; }
        
    private:
        void* m_q_buffer{};
        size_t m_size{};
    };

    class cq : public ring_queue
    {
        friend class uring_storage;
        cq(void* ring_buffer, size_t size) noexcept
            : ring_queue{ ring_buffer, size }
        {
        }

    public:

    private:
    };
    
    class sq : public ring_queue
    {
        friend class uring_storage; 
        sq(void* ring_buffer, size_t size, void* sqes) noexcept 
            : ring_queue{ ring_buffer, size }
        {
            m_sqes = static_cast<::io_uring_sqe*>(sqes);
        }

    public:
        
    private:
        ::io_uring_sqe* m_sqes{};
    };
}

#endif
