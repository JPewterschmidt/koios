#ifndef KOIOS_EXCEPTIONS_H
#define KOIOS_EXCEPTIONS_H

#include <stdexcept>
#include <exception>
#include <source_location>
#include <cstring>
#include <string>

#include "fmt/core.h"

#include "koios/macros.h"
#include "koios/error_category.h"

KOIOS_NAMESPACE_BEG

/*! \brief The most fundamental exception type related koios.
 *  
 *  Every koios runtime related exception will inherited from this class.
 */
class exception : public ::std::exception
{
public:
    exception() = default;

    exception(::std::string_view msg) noexcept
        : m_msg{ msg }
    {
    }

    exception(::std::error_code ec) noexcept
        : m_msg{ ec.message() }
    {
    }

    virtual const char* what() const noexcept override { return m_msg.c_str(); }
    void log() const noexcept;

protected:
    ::std::string m_msg;
};

/*! \brief Exception which will be thrown 
 *         when user want to enqueue a new functor 
 *         into a `thread_pool` which has called `quick_stop`.
 */
class thread_pool_stopped_exception : public koios::exception
{
public:
    thread_pool_stopped_exception()
        : koios::exception("thread_pool has been stopped, now stop try to enqueue something else.")
    {
    }

    thread_pool_stopped_exception(auto&& msg)
        : koios::exception(::std::forward<decltype(msg)>(msg))
    {
    }
};

/*! \brief Similar to `thread_pool_stopped_exception`. only thrown in `runtime.cc` file. */
class runtime_not_working_exception : public thread_pool_stopped_exception
{
public:
    runtime_not_working_exception(::std::source_location sl = ::std::source_location::current()) noexcept
        : thread_pool_stopped_exception{ 
            fmt::format("runtime_not_working_exception: {}:{}:{}", 
                        sl.file_name(), sl.line(), sl.function_name()) 
          }
    {
    }
};

class uring_exception : public koios::exception
{
public:
    uring_exception() = default;

    uring_exception(::std::string_view msg) noexcept
        : koios::exception{ msg }
    {
    }

    uring_exception(int errno) noexcept
        : uring_exception{ ::strerror(errno) }
    {
    }

    uring_exception(::std::error_code ec) noexcept
        : uring_exception{ ec.message() }
    {
    }

    bool has_error_code() const noexcept { return m_has_ec; }
    ::std::error_code error_code() const noexcept { return m_ec; }

private:
    ::std::error_code m_ec;
    bool m_has_ec;
};

class future_exception : public koios::exception
{
public:
    future_exception() = default;
    const char* what() const noexcept override { return "nothing to get."; }   
};

class socket_exception : public koios::uring_exception
{
public:
    socket_exception(::std::error_code ec) noexcept
        : koios::uring_exception{ ::std::move(ec) }
    {
    }
};

void log_info(::std::string msg);
void log_error(::std::string msg);
void log_debug(::std::string msg);

class stream_buffer_exception : public koios::exception
{
private:
    stream_buffer_exception(::std::string_view msg) noexcept
        : koios::exception{ msg }
    {
    }

public:
    static stream_buffer_exception 
    make_stream_buffer_exception_read_overflow()
    {
        return { "Streambuffer: Read overflow!" };
    }

    static stream_buffer_exception
    make_stream_buffer_exception_write_overflow()
    {
        return { "Streambuffer: Write overflow!" };
    }
};

class expected_exception : public koios::exception
{
public:
    expected_exception(int err) noexcept 
        : koios::exception{ ::std::error_code{ err, koios::expected_category() } }
    {
    }

    expected_exception(::std::error_code ec) noexcept
        : koios::exception{ ::std::move(ec) }
    {
    }
};

KOIOS_NAMESPACE_END

#endif
