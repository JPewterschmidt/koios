/* Koios, A c++ async runtime library.
 * Copyright (C) 2024  Jeremy Pewterschmidt
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KOIOS_WORK_STEALING_QUEUE_H
#define KOIOS_WORK_STEALING_QUEUE_H

#include <unordered_map>
#include <thread>
#include <shared_mutex>
#include <mutex>
#include <unordered_set>
#include <ranges>
#include <atomic>

#include "koios/macros.h"
#include "koios/queue_concepts.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/per_consumer_attr.h"

KOIOS_NAMESPACE_BEG

template<invocable_queue_concept LockFreeInvocableQueue>
class work_stealing_queue
{
public:
    using queue_type = LockFreeInvocableQueue;
    using invocable_type = typename LockFreeInvocableQueue::invocable_type;

public:
    work_stealing_queue() = default;
    work_stealing_queue(work_stealing_queue&& q) noexcept
        : m_queues{ q.m_queues.exchange(nullptr) }
    {
    }

    ::std::optional<invocable_type> 
    dequeue(const per_consumer_attr& attr)
    {
        auto qs = this->queues_ptr();
        auto& queues = *qs;
        ::std::optional<invocable_type> result{ 
            queues[attr.thread_id]->dequeue()
        };

        if (!result)
        {
            for (auto& [k, q] : queues)
            {
                if ((result = q->dequeue()))
                    break;
            }
        }
        return result;
    }

    void enqueue(invocable_type i)
    {
        this->enqueue({}, ::std::move(i));
    }

    void enqueue(const per_consumer_attr& ca, invocable_type i)
    {
        auto tid = ca.thread_id;
        auto qs = this->queues_ptr();
        auto& queues = *qs;
        if (!queues.contains(tid))
        {
            tid = (queues.begin())->first;
        }
        queues.at(tid)->enqueue(::std::move(i));
    }

    size_t size() const noexcept
    {
        size_t result{};
        auto qs = this->queues_ptr();
        auto& queues = *qs;
        for (const auto& [k, q] : queues)
        {
            result += q->size();
        }
        return result;
    }

    bool empty() const noexcept { return this->size() == 0; }

    void thread_specific_preparation(const per_consumer_attr& attr)
    {
        this->add_consumer_thread_id(attr);
    }

private:
    using queue_type_ptr        = ::std::shared_ptr<queue_type>;
    using queues_hashmap        = ::std::unordered_map<::std::thread::id, queue_type_ptr>;
    using queues_hashmap_ptr    = ::std::shared_ptr<queues_hashmap>;

    auto queues_ptr() const
    {
        queues_hashmap_ptr qs_ptr;
        while (!(qs_ptr = m_queues.load()))
            ;
        return qs_ptr;
    }

    static queues_hashmap_ptr make_queues_hashmap() { return ::std::make_shared<queues_hashmap>();  }
    static queue_type_ptr make_queue() { return ::std::make_shared<queue_type>(); }
    static queues_hashmap_ptr dup_queues(queues_hashmap_ptr q) { return ::std::make_shared<queues_hashmap>(*q); }

    void add_consumer_thread_id(::std::thread::id tid)
    {
        queues_hashmap_ptr old_hashmap;
        while (!(old_hashmap = m_queues.exchange(nullptr)))
            ;
        queues_hashmap_ptr new_hashmap = this->dup_queues(old_hashmap);
        new_hashmap->insert({tid, this->make_queue()});
        m_queues.store(::std::move(new_hashmap));
    }

    void add_consumer_thread_id(const per_consumer_attr& attr)
    {
        this->add_consumer_thread_id(attr.thread_id);
    }

private:
    ::std::atomic<queues_hashmap_ptr> m_queues{ this->make_queues_hashmap() };
};

KOIOS_NAMESPACE_END

#endif
