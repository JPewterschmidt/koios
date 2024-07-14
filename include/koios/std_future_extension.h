// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_FUTURE_H
#define KOIOS_FUTURE_H

#include <chrono>
#include <future>

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

template<typename T>
class future_extension : public ::std::future<T>
{
public:
    future_extension(::std::future<T>&& stdf) noexcept
        : ::std::future<T>{ ::std::move(stdf) }
    {
    }

    bool ready()
    {
        switch (this->wait_for(::std::chrono::nanoseconds{0}))
        {
            case ::std::future_status::ready: return true;
        }
        return false;
    }
};

KOIOS_NAMESPACE_END

#endif
