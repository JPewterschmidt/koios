#ifndef KOIOS_STREAM_BUFFER_H
#define KOIOS_STREAM_BUFFER_H

#include "koios/macros.h"
#include "toolpex/allocator_guard.h"
#include "toolpex/move_only.h"
#include "koios/exceptions.h"

#include <memory>
#include <cstddef>
#include <vector>
#include <span>

KOIOS_NAMESPACE_BEG

template<typename T = ::std::byte, typename Alloc = ::std::allocator<T>>
class stream_buffer : public toolpex::move_only
{
public:
    using allocator_type = Alloc;
    using value_type = typename ::std::allocator_traits<allocator_type>::value_type;
    using pointer = typename ::std::allocator_traits<allocator_type>::pointer;

    static_assert(sizeof(value_type) == sizeof(::std::byte), 
                  "the buffer value_type should only be a fundamental type ,"
                  "which sizeof(value_type) == sizeof(::std::byte)!");

    stream_buffer(size_t block_size_nbytes = 1024)
        : m_block_size{ block_size_adjust(block_size_nbytes) }
    {
        const pointer first_block{ 
            ::std::allocator_traits<allocator_type>::allocate(
                m_alloc, this->block_size())
        };
        m_blocks.emplace_back(m_alloc, first_block);
        m_commiter_cursor = cursor{ first_block, m_block_size };
        m_consumer_cursor = cursor{ first_block, m_block_size };
    }

    stream_buffer(stream_buffer&&) noexcept = default;
    stream_buffer& operator=(stream_buffer&&) noexcept = default;

    /*! \brief  Commits bytes filled, and get new writable memory span.
     *  \retval the new writable memory.
     *  \param  bytes_filled The bytes you have filled into a writable span 
     *                       that returned by the previous call.
     *                       if this parameter is 0, 
     *                       it will return the same span as the previous call.
     *                       This parameter should be 0, 
     *                       when your first call of this member function.
     *
     *  \attention The value of parameter `bytes_filled` should not exceed the bytes-size 
     *             of the span object returned by previous call.
     *             Or it will throw a `stream_buffer_exception`
     *  \see `koios::stream_buffer_exception`
     *
     *  This function would potentially allocate new space.
     */
    ::std::span<value_type> commit(size_t bytes_filled)
    {
        if (m_commiter_cursor.left() < bytes_filled || bytes_filled > this->block_size())
            throw stream_buffer_exception::make_stream_buffer_exception_write_overflow();

        auto ret = m_commiter_cursor.next_chunk(bytes_filled);
        if (ret.size_bytes() != 0) // Current block was filled completely.
            return ret;

        // Allocate new block
        auto& block = m_blocks.emplace_back(
            toolpex::allocate_guarded(m_alloc, this->block_size())
        );

        m_commiter_cursor = { block, this->block_size() };
        return m_commiter_cursor.next_chunk(0);
    }

    /*! \brief  Consumed bytes readed, and get new readable memory span.
     *  \retval the new readable memory.
     *  \param  bytes_readed The bytes you have readed from a readable span 
     *                       that returned by the previous call.
     *                       if this parameter is 0, 
     *                       it will return the same span as the previous call.
     *                       This parameter should be 0, 
     *                       when your first call of this member function.
     *
     *  \attention The value of parameter `bytes_readed` should not exceed the bytes-size 
     *             of the span object returned by previous call.
     *             Or it will throw a `stream_buffer_exception`
     *  \see `koios::stream_buffer_exception`
     */
    ::std::span<const value_type> consume(size_t bytes_readed)
    {
        if (m_commiter_cursor.left() < bytes_readed || bytes_readed > this->block_size())
            throw stream_buffer_exception::make_stream_buffer_exception_read_overflow();

        auto ret = m_commiter_cursor.next_chunk(bytes_readed);
        if (ret.size_bytes() != 0) // Current block was filled completely.
            return ret;

        m_consumer_cursor = { 
            m_blocks[++m_block_consumer_pointed], 
            this->block_size()
        };
        return m_consumer_cursor.next_chunk(0);
    }

protected:
    size_t block_size() const noexcept { return m_block_size; }

    // Should block size align to 4K?
    static size_t block_size_adjust(size_t size) noexcept { return size; } 

    class cursor : toolpex::move_only
    {
    private:
        pointer m_cursor{};
        size_t m_left_bytes;

    public:
        constexpr cursor() = default;

        cursor(pointer blockbeg, size_t nbytes) noexcept
            : m_cursor{ blockbeg }, m_left_bytes{ nbytes }
        {
        }

        /*! \attention This function wont do any out_of_range check! */
        ::std::span<value_type> next_chunk(size_t bytes_used) noexcept
        {
            return { m_cursor += bytes_used, m_left_bytes -= bytes_used };
        }

        size_t left() const noexcept { return m_left_bytes; }
    };

private:
    const size_t m_block_size{};
    allocator_type m_alloc;
    ::std::vector<toolpex::allocator_ptr<allocator_type>> m_blocks;
    cursor m_commiter_cursor{};
    cursor m_consumer_cursor{};
    size_t m_block_consumer_pointed{0};
};

KOIOS_NAMESPACE_END

#endif
