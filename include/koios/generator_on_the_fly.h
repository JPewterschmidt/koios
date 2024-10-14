// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_GENERATOR_ON_THE_FLY_H
#define KOIOS_GENERATOR_ON_THE_FLY_H

#include <coroutine>
#include <utility>

#include "macros.h"

KOIOS_NAMESPACE_BEG

class generator_on_the_fly
{
public:
    constexpr generator_on_the_fly() noexcept = default;

    generator_on_the_fly(::std::coroutine_handle<> h) 
        : m_h{ h }
    {
    }

    template<typename T>
    generator_on_the_fly(::std::coroutine_handle<T> h)
        : m_h{ h }
    {
    }

    generator_on_the_fly(generator_on_the_fly&& other) noexcept = default;
    generator_on_the_fly& operator=(generator_on_the_fly&& other) noexcept = default;

    generator_on_the_fly(const generator_on_the_fly&) = delete;
    generator_on_the_fly& operator=(const generator_on_the_fly&) = delete;

    /*! \brief Execute and give up the ownership of the related `std::coroutine_handle`.
     *
     *  This member function will firstly release the ownership of the handler, 
     *  secondly call the `std::coroutine_handle::resume()` of the handler, 
     */
    void operator()()
    { 
        m_h.resume(); 
        m_h = nullptr;
    }

    operator bool() const noexcept { return !!address(); }
    bool valid() const noexcept { return this->operator bool(); }

    bool done() const noexcept
    {
        if (m_h) [[likely]] return m_h.done();
        return true;
    }

    void* address() const noexcept
    {
        if (!m_h) return nullptr;
        return m_h.address();
    }

private:
    ::std::coroutine_handle<> m_h;
};

KOIOS_NAMESPACE_END

#endif
