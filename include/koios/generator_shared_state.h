#ifndef KOIOS_GENERATOR_SHARED_STATE_H
#define KOIOS_GENERATOR_SHARED_STATE_H

#include "toolpex/object_storage.h"
#include "toolpex/spin_lock.h"

#include "koios/task_on_the_fly.h"
#include "koios/waiting_handle.h"

namespace koios::generator_details
{

template<typename T>
class shared_state
{
private:
    task_on_the_fly m_generator_coro;
    task_on_the_fly m_waiting_coro;
    toolpex::object_storage<T> m_yielded_val;
    ::std::atomic_bool m_finalized{};
    mutable toolpex::spin_lock m_lock;

public:
    bool has_value() const noexcept
    {
        ::std::lock_guard _{ m_lock };
        return !m_finalized.load(::std::memory_order_acquire) && m_yielded_val.has_value();
    }

    void set_finalized() noexcept 
    { 
        m_finalized.store(true, ::std::memory_order_release);
    }

    bool finalized() const noexcept 
    { 
        return m_finalized.load(::std::memory_order_acquire);
    }

    template<typename TT>
    void set_value(TT&& val)
    {
        ::std::lock_guard _{ m_lock };
        m_yielded_val.set_value(::std::forward<TT>(val));
    }

    void set_generator_coro(task_on_the_fly f) noexcept
    {
        ::std::lock_guard _{ m_lock };
        toolpex_assert(!m_generator_coro);
        m_generator_coro = ::std::move(f);
    }

    auto get_generator_coro() noexcept
    {
        ::std::lock_guard _{ m_lock };
        toolpex_assert(!!m_generator_coro);
        return ::std::move(m_generator_coro);
    }

    void set_waiting_coro(task_on_the_fly f) noexcept
    {
        ::std::lock_guard _{ m_lock };
        toolpex_assert(!m_waiting_coro);
        m_waiting_coro = ::std::move(f);
    }

    auto get_waiting_coro() noexcept
    {
        ::std::lock_guard _{ m_lock };
        // If there're no one still waiting for the generator, then nothing to return. 
        // Thus no need to check whether there's waiting coro handler.
        return ::std::move(m_waiting_coro);
    }

    bool has_waiting_coro() const noexcept 
    { 
        ::std::lock_guard _{ m_lock };
        return !!m_waiting_coro; 
    }

    bool has_generator_coro() const noexcept 
    { 
        ::std::lock_guard _{ m_lock };
        return !!m_generator_coro; 
    }

    void try_finalize() noexcept
    {
        ::std::lock_guard _{ m_lock };
        toolpex_assert(!m_waiting_coro);
        m_finalized = true;
        if (!!m_generator_coro)
            m_generator_coro = {};
    }

    // This two functions bwlow are not necessaryly to be protected by lock, 
    // since there's only one thread could yield value once a time.
    auto& yielded_value_slot() noexcept { return m_yielded_val; }
    const auto& yielded_value_slot() const noexcept { return m_yielded_val; }
};

template<typename T>
using shared_state_sptr = ::std::shared_ptr<shared_state<T>>;

} // namespace koios::generator_details

#endif
