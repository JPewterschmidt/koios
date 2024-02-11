#ifndef KOIOS_IOURING_CANCEL_AW_H
#define KOIOS_IOURING_CANCEL_AW_H

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include "toolpex/unique_posix_fd.h"
#include <coroutine>

namespace koios::uring
{
    class cancel_first : public iouring_aw 
    { 
    public: 
        cancel_first(::std::coroutine_handle<> handle); 
        cancel_first(void* handle); 
        cancel_first(uint64_t handle); 
    };

    class cancel_all : public iouring_aw 
    { 
    public: 
        cancel_all(::std::coroutine_handle<> handle); 
        cancel_all(void* handle); 
        cancel_all(uint64_t handle); 
    };

    class cancel_any : public iouring_aw 
    { 
    public: 
        cancel_any(const toolpex::unique_posix_fd& fd, ::std::coroutine_handle<> handle); 
        cancel_any(const toolpex::unique_posix_fd& fd, void* handle); 
        cancel_any(const toolpex::unique_posix_fd& fd, uint64_t handle); 
        cancel_any(int fd, ::std::coroutine_handle<> handle);
        cancel_any(int fd, void* handle); 
        cancel_any(int fd, uint64_t handle); 
    };
}

#endif
