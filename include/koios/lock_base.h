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
          m_hold{ ::std::exchange(other.m_hold, false) }
    {
    }

    lock_base& operator=(lock_base&& other) noexcept
    {
        unlock();

        m_mutex = ::std::exchange(other.m_mutex, nullptr);
        m_hold = ::std::exchange(other.m_hold, false);

        return *this;
    }

    /*! \brief Give up the ownership of the corresponding mutex.
     *  
     *  After give up the ownership, you can `lock()`
     *  corresponding the corresponding mutex.
     */
    void unlock() noexcept
    {
        if (m_mutex && is_hold())
        {
            m_mutex->release();
            m_hold = false;
        }
    }

    ~lock_base() noexcept { unlock(); }

    bool is_hold() const noexcept { return m_hold; }

protected:
    Mutex* m_mutex{};
    bool m_hold{ true };
};

} // namespace koios

#endif
