#ifndef KOIOS_WAIT_GROUP_H
#define KOIOS_WAIT_GROUP_H

#include <utility>
#include <atomic>

#include "koios/waiting_handle.h"

namespace koios
{

class wait_group
{
public:
    void add(size_t num = 1)
    {
        m_count.fetch_add(num, ::std::memory_order_relaxed);
    }

    void done(size_t i = 1)
    {
        if (m_count.fetch_sub(i, ::std::memory_order_acq_rel) == 1)
        {
            wake_up_all();
        }
    }

    bool ready() const noexcept
    {
        return m_count.load(::std::memory_order_acquire) == 0;
    }

    auto wait()
    {
        struct wait_aw
        {
            wait_group* m_parent{};

            bool await_ready() const noexcept
            {
                return m_parent->ready();
            }

            void await_suspend(task_on_the_fly t)
            {
                m_parent->m_waitings.enqueue({ .task = ::std::move(t) });
            }

            constexpr void await_resume() const noexcept {}
        };
        
        return wait_aw{ this };
    }

private:
    void wake_up_all()
    {
        waiting_handle t;
        while (m_waitings.try_dequeue(t))
        {
            wake_up(t);   
        }
    }

private:
    ::std::atomic_size_t m_count;
    moodycamel::ConcurrentQueue<waiting_handle> m_waitings;
};

class wait_group_guard
{
public:
    constexpr wait_group_guard() noexcept = default;

    wait_group_guard(wait_group& p, size_t i = 1) noexcept
        : m_parent{ &p }, m_count{ i }
    {
        m_parent->add(m_count);
    }

    wait_group_guard(wait_group_guard&& other) noexcept
        : m_parent{ ::std::exchange(other.m_parent, nullptr) }, 
          m_count{ other.m_count }
    {
    }

    wait_group_guard& operator=(wait_group_guard&& other) noexcept
    {
        clear();
        m_parent = ::std::exchange(other.m_parent, nullptr);
        m_count = other.m_count;
        return *this;
    }

    ~wait_group_guard() noexcept
    {
        if (!m_parent)
            return;

        m_parent->done(m_count);
        clear();
    }

    void clear() noexcept
    {
        m_parent = nullptr;
    }

private:
    wait_group* m_parent{};
    size_t m_count{};
};

} // namespace koios

#endif
