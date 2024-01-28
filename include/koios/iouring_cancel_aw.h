#ifndef KOIOS_IOURING_CANCEL_AW_H
#define KOIOS_IOURING_CANCEL_AW_H

#include "koios/macros.h"
#include "koios/iouring_aw.h"
#include <coroutine>

namespace koios::uring
{
    class cancel_first : public iouring_aw 
    { 
    public: 
        cancel_first(::std::coroutine_handle<> handle); 
        cancel_first(void* handle); 
    };

    class cancel_all : public iouring_aw 
    { 
    public: 
        cancel_all(::std::coroutine_handle<> handle); 
        cancel_all(void* handle); 
    };
}

#endif
