// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_LOCK_BASE_H
#define KOIOS_LOCK_BASE_H

#include "toolpex/move_only.h"

namespace koios
{

template<typename Mutex>
class lock_base : public toolpex::move_only
{
public:
    lock_base(Mutex& m) noexcept
        : m_mutex{ &m }
    {
    }

    lock_base() noexcept = default;

    lock_base(lock_base&& other) noexcept 
        : m_mutex{ ::std::exchange(other.m_mutex, nullptr) }, 
          m_owns{ ::std::exchange(other.m_owns, false) }
    {
    }

    lock_base& operator=(lock_base&& other) noexcept
    {
        this->unlock();

        m_mutex = ::std::exchange(other.m_mutex, nullptr);
        m_owns = ::std::exchange(other.m_owns, false);

        return *this;
    }

    /*! \brief Give up the ownership of the corresponding mutex.
     *  
     *  After give up the ownership, you can `lock()`
     *  corresponding the corresponding mutex.
     */
    void unlock() noexcept
    {
        if (m_mutex && this->owns_lock())
        {
            m_mutex->release();
            m_owns = false;
        }
    }

    ~lock_base() noexcept { this->unlock(); }

    bool owns_lock() const noexcept { return m_owns; }

protected:
    Mutex* m_mutex{};
    bool m_owns{ true };
};

} // namespace koios

#endif
