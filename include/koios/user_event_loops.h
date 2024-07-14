// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef KOIOS_USER_EVENT_LOOPS_H
#define KOIOS_USER_EVENT_LOOPS_H

#include "koios/macros.h"
#include "koios/per_consumer_attr.h"
#include "koios/user_event_loop_interface.h"

#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <unordered_map>
#include <thread>

KOIOS_NAMESPACE_BEG

class user_event_loops
{
public:
    void thread_specific_preparation(const per_consumer_attr& attr) noexcept;
    void stop() noexcept;
    void quick_stop() noexcept;
    void until_done();
    ::std::chrono::milliseconds max_sleep_duration(const per_consumer_attr& attr) noexcept;
    void do_occured_nonblk() noexcept;
    void add_loop(user_event_loop_interface::sptr loop);

private:
    ::std::unordered_map<::std::thread::id, const per_consumer_attr*> m_attrs;
    ::std::vector<user_event_loop_interface::sptr> m_loops;
    bool m_cleanning{};
    mutable ::std::shared_mutex m_mutex;
};

KOIOS_NAMESPACE_END

#endif
