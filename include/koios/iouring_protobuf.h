#ifndef KOIOS_IOURING_PROTOBUF_H
#define KOIOS_IOURING_PROTOBUF_H

#include <chrono>
#include <array>

#include "toolpex/encode.h"
#include "toolpex/unique_posix_fd.h"
#include "toolpex/concepts_and_traits.h"

#include "koios/task.h"
#include "koios/iouring_awaitables.h"

namespace koios::uring
{

template<typename PbMsg>
concept protobuf_msg_concept = requires (PbMsg p)
{
    { p.IsInitialized() } -> toolpex::boolean_testable;
    { p.SerializeAsString() } -> ::std::same_as<::std::string>;
    { p.ParseFromArray(::std::declval<void*>(), ::std::declval<int>()) } -> toolpex::boolean_testable;
};

task<bool> send_pb_message(const toolpex::unique_posix_fd& fd, protobuf_msg_concept auto const& pb)
{
    if (!pb.IsInitialized()) co_return false;

    const auto msg_rep = pb.SerializeAsString();
    auto left = ::std::as_bytes(::std::span{ msg_rep });
    size_t wrote{};
    
    // Send message length prefix with big endian.
    ::std::array<::std::byte, sizeof(uint32_t)> msg_len_buf{};
    toolpex::encode_big_endian_to(static_cast<uint32_t>(msg_rep.size()), msg_len_buf);

    size_t retry_count{};
    ioret_for_data_deliver prefix_sent_ret{};
    do
    {
        ++retry_count;
        prefix_sent_ret = co_await uring::send(fd, msg_len_buf);
        if (retry_count > 5) co_return false;
    }
    while (is_timeout_ec(prefix_sent_ret.error_code()));
    retry_count = 0;
    
    // Send message body.
    while (!left.empty())
    {
        if (retry_count > 10) break;
        auto ret = co_await uring::send(fd, left);
        if (auto ec = ret.error_code(); ec && !is_timeout_ec(ec))
        {
            break;
        }
        else if (!ec)
        {
            const auto cur_wrote = ret.nbytes_delivered();
            left = left.subspan(cur_wrote);
            wrote += cur_wrote;
        }
        else
        {
            ++retry_count;
        }
    }
    
    co_return true;
}

task<bool> recv_pb_message(const toolpex::unique_posix_fd& fd, protobuf_msg_concept auto& msg)
{
    using namespace ::std::chrono_literals;
    ::std::array<::std::byte, sizeof(uint32_t)> prefix_buf{};
    ::std::error_code ec{};
    co_await recv_fill_buffer(fd, prefix_buf, 0, ec, 1min);
    if (ec) co_return false;

    const uint32_t prefix_len = toolpex::decode_big_endian_from<uint32_t>(prefix_buf);
    auto buf = ::std::unique_ptr<::std::byte[]>{ 
        // TODO: remove default init (fill zero) after test
        new ::std::byte[prefix_len]{} 
    };
    ::std::span<::std::byte> writable{ buf.get(), prefix_len };
    const size_t received = co_await recv_fill_buffer(fd, writable, 0, ec, 3min);
    if (ec || received != prefix_len)
    {
        co_return false;
    }
    co_return msg.ParseFromArray(writable.data(), static_cast<int>(writable.size()));
}

} // namespace koios::uring

#endif
