#ifndef KOIOS_WAIT_GROUP_H
#define KOIOS_WAIT_GROUP_H

#include <utility>
#include <atomic>

#include "koios/waiting_handle.h"

namespace koios
{

class wait_group_handle;

class wait_group
{
public:
    wait_group_handle add(size_t num = 1);

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
    void wake_up_all();

private:
    ::std::atomic_size_t m_count;
    moodycamel::ConcurrentQueue<waiting_handle> m_waitings;
};

class wait_group_handle
{
public:
    constexpr wait_group_handle() noexcept = default;

    wait_group_handle(wait_group* p) noexcept
        : m_parent{ p }
    {
    }

    wait_group_handle(wait_group_handle&& other) noexcept
        : m_parent{ ::std::exchange(other.m_parent, nullptr) }
    {
    }

    wait_group_handle& operator=(wait_group_handle&& other) noexcept
    {
        clear();
        m_parent = ::std::exchange(other.m_parent, nullptr);
        return *this;
    }

    ~wait_group_handle() noexcept
    {
        if (!m_parent)
            return;

        m_parent->done();
        clear();
    }

    void clear() noexcept
    {
        m_parent = nullptr;
    }

private:
    wait_group* m_parent{};
};

} // namespace koios

#endif
