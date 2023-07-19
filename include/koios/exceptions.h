#ifndef KOIOS_EXCEPTIONS_H
#define KOIOS_EXCEPTIONS_H

#include <stdexcept>
#include <exception>
#include <source_location>

#include "fmt/core.h"

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

class exception : public ::std::exception
{
public:
    exception(auto&& msg)
        : m_msg{ ::std::forward<decltype(msg)>(msg) }
    {
    }

    const char* what() const noexcept { return m_msg.c_str(); }

protected:
    ::std::string m_msg;
};

class thread_pool_stopped_exception : public koios::exception
{
public:
    thread_pool_stopped_exception(auto&& msg)
        : koios::exception(::std::forward<decltype(msg)>(msg))
    {
    }
};

class runtime_shutdown_exception : public thread_pool_stopped_exception
{
public:
    runtime_shutdown_exception(::std::source_location sl = ::std::source_location::current()) noexcept
        : thread_pool_stopped_exception{ 
            fmt::format("runtime_shutdown_exception: {}:{}:{}", 
                        sl.file_name(), sl.line(), sl.function_name()) 
          }
    {
    }
};

KOIOS_NAMESPACE_END

#endif
