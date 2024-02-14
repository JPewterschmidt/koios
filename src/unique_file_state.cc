/* koios, a c++ async runtime library.
 * copyright (c) 2024  jeremy pewterschmidt
 * 
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 * 
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 * 
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 */

#include "koios/unique_file_state.h"
#include "toolpex/exceptions.h"
#include "koios/error_category.h"
#include "koios/iouring_awaitables.h"
#include <cerrno>

KOIOS_NAMESPACE_BEG

unique_file_state::
unique_file_state(::std::filesystem::path path)
    : m_path{ ::std::move(path) }, 
      m_fd{ ::open(path.c_str(), O_APPEND) }
{
    if (!m_fd.valid()) [[unlikely]]
        throw toolpex::posix_exception{ errno };
}

task<::std::error_code> 
unique_file_state::
append_async(::std::span<const ::std::byte> buffer) noexcept
try 
{
    size_t left = buffer.size_bytes();
    ::std::error_code ec{};

    auto lk = co_await m_fs_io_lock.acquire();
    
    while (left > 0)
    {
        auto ret = co_await uring::write(m_fd, buffer);
        if ((ec = ret.error_code())) break;
        left -= ret.nbytes_delivered();
    }

    co_return ec;
}
catch (const uring_exception& e)
{
    if (e.has_error_code())
        co_return e.error_code();
    co_return { KOIOS_EXCEPTION_CATCHED, koios_category() };
}

KOIOS_NAMESPACE_END
