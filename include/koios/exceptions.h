#ifndef KOIOS_EXCEPTIONS_H
#define KOIOS_EXCEPTIONS_H

#include <stdexcept>
#include <exception>
#include <source_location>

#include "fmt/core.h"

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

/*! \brief The most fundamental exception type related koios.
 *  
 *  Every koios runtime related exception will inherited from this class.
 */
class exception : public ::std::exception
{
public:
    exception() = default;

    exception(auto&& msg)
        : m_msg{ ::std::forward<decltype(msg)>(msg) }
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

void log_info(::std::string msg);
void log_error(::std::string msg);
void log_debug(::std::string msg);

KOIOS_NAMESPACE_END

#endif
