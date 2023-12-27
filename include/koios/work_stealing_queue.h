#ifndef KOIOS_WORK_STEALING_QUEUE_H
#define KOIOS_WORK_STEALING_QUEUE_H

#include <unordered_map>
#include <thread>
#include <shared_mutex>
#include <mutex>
#include <unordered_set>
#include <ranges>

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
        : m_queues{ ::std::move(q.m_queues) },
          m_consumers{ ::std::move(q.m_consumers) }
    {
    }

    ::std::optional<invocable_type> 
    dequeue(const per_consumer_attr& attr)
    {
        ::std::shared_lock lk{ m_queues_lk };
        ::std::optional<invocable_type> result{ 
            m_queues[attr.thread_id].dequeue()
        };

        if (!result)
        {
            for (auto& [k, q] : m_queues)
            {
                if ((result = q.dequeue()))
                    break;
            }
        }
        return result;
    }

    void enqueue(invocable_type i)
    {
        enqueue({}, ::std::move(i));
    }

    void enqueue(const per_consumer_attr& ca, invocable_type i)
    {
        
        auto tid = ca.thread_id;
        ::std::shared_lock lk{ m_queues_lk };
        if (!m_consumers.contains(tid))
        {
            tid = m_queues.begin()->first;
        }
        m_queues[tid].enqueue(::std::move(i));
    }

    size_t size() const noexcept
    {
        size_t result{};
        ::std::shared_lock lk{ m_queues_lk };
        for (const auto& [k, q] : m_queues)
        {
            result += q.size();
        }
        return result;
    }

    bool empty() const noexcept { return size() == 0; }

    void thread_specific_preparation(const per_consumer_attr& attr)
    {
        add_consumer_thread_id(attr);
    }

private:
    void add_consumer_thread_id(::std::thread::id tid)
    {
        ::std::unique_lock lk{ m_queues_lk };
        m_consumers.insert(tid);
        m_queues[tid] = queue_type{};
    }

    void add_consumer_thread_id(const per_consumer_attr& attr)
    {
        add_consumer_thread_id(attr.thread_id);
    }

private:
    ::std::unordered_map<::std::thread::id, queue_type> m_queues;
    mutable ::std::shared_mutex m_queues_lk;
    ::std::unordered_set<::std::thread::id> m_consumers;
};

KOIOS_NAMESPACE_END

#endif
