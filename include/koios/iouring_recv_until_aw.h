#ifndef KOIOS_IOUIRNG_RECV_UNTIL_AW_H
#define KOIOS_IOUIRNG_RECV_UNTIL_AW_H

#include "koios/stream_buffer.h"
#include "koios/iouring_recv_aw.h"
#include "toolpex/exceptions.h"

namespace koios::uring
{
    /*! \brief  Task which can continuously call uring::recv
     *          until there's subrange of the parameter `buffer`
     *          has the same contents as the parameter `slice` has.
     *  \return the index of bytes of  
     */
    template<typename T>
    task<size_t> 
    recv_until(const toolpex::unique_posix_fd& fd, 
               stream_buffer<T>& buffer, 
               ::std::span<const T> slice)
    try
    {
        using value_type = T;
        using buffer_t = typename task<stream_buffer<T>>::value_type;

        auto buf_ctx = buffer.get_buffer_context();
        // Save the buffer consumer information.
        const auto buffer_ctx = buffer.get_consumer_cursor();
    
        typename buffer_t::immutable_span_type previouse_readed{};
        const auto matched = [&previous_received](auto cur, size_t* readed_ret) { 
            // ...
            previous_received = cur;

            // TODO ****************************
            toolpex::not_implemented();
            // TODO ****************************

            size_t readed{};
            if (previous_received.size_bytes() == 0) // first call
            {
            }

            *readed_ret = readed;
            return false;
        };    

        size_t readed{};
        auto writable_buffer = buffer.commit(0);
        
        do
        {
            auto current_received = buffer.consume(readed);
            auto recvret = co_await uring::recv(
                fd, as_writable_bytes(writable_buffer)
            );

            if (auto ec = recvret.error_code(); ec)
                throw uring_exception{ ec };
            readed = recvret.nbytes_delivered();
            writable_buffer = buffer.commit(readed);
        }
        while (!matched(current_received, &readed));

        // Recovery the buffer ctx as before any call
        buffer.reset_consumer(buffer_ctx);
        co_return buffer;
    }
    catch (...)
    {
        buffer.set_buffer_context(ctx);      
        throw;
    }
}

#endif
