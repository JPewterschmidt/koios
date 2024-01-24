#ifndef KOIOS_USER_EVENT_LOOPS_H
#define KOIOS_USER_EVENT_LOOPS_H

#include "koios/macros.h"
#include "koios/per_consumer_attr.h"

#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <vector>

KOIOS_NAMESPACE_BEG

class user_event_loop
{
public:
    using uptr = ::std::unique_ptr<user_event_loop>;

public:
    virtual void thread_specific_preparation(const per_consumer_attr& attr) noexcept = 0;
    virtual void stop() noexcept = 0;
    virtual void quick_stop() noexcept = 0;
    virtual void until_done() = 0;
    virtual ::std::chrono::milliseconds max_sleep_duration(const per_consumer_attr& attr) noexcept = 0;
    virtual void do_occured_nonblk() noexcept = 0;
};

class user_event_loops
{
public:
    void thread_specific_preparation(const per_consumer_attr& attr) noexcept;
    void stop() noexcept;
    void quick_stop() noexcept;
    void until_done();
    ::std::chrono::milliseconds max_sleep_duration(const per_consumer_attr& attr) noexcept;
    void do_occured_nonblk() noexcept;
    void add_loop(user_event_loop::uptr loop);

private:
    ::std::vector<user_event_loop::uptr> m_loops;
    mutable ::std::shared_mutex m_mutex;
    bool m_after_ts_prep{ false };
};

KOIOS_NAMESPACE_END

#endif
