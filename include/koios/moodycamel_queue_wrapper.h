#ifndef KOIOS_MOODYCAMEL_QUEUE_WRAPPER_H
#define KOIOS_MOODYCAMEL_QUEUE_WRAPPER_H

#include <functional>
#include <optional>

#include "koios/macros.h"
#include "koios/per_consumer_attr.h"

#include "concurrentqueue/concurrentqueue.h"

KOIOS_NAMESPACE_BEG

/*! \brief The moodycamel Concurrent queue wrapper. 
 *         To satisfy the `invocable_queue_wrapper`.
 *  This is a thread-safe class.
 */
class moodycamel_queue_wrapper
{
public:
    using invocable_type = ::std::move_only_function<void()>;
    using queue_type = moodycamel::ConcurrentQueue<invocable_type>;

public:
    void enqueue(invocable_type&& func);
    ::std::optional<invocable_type> dequeue();
    bool empty() const;
    size_t size() const noexcept { return m_q.size_approx(); }

private:
    queue_type m_q;
};

KOIOS_NAMESPACE_END

#endif
