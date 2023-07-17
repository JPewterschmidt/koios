#ifndef KOIOS_MOODYCAMEL_QUEUE_WRAPPER_H
#define KOIOS_MOODYCAMEL_QUEUE_WRAPPER_H

#include <functional>
#include <optional>

#include "macros.h"

#include "concurrentqueue/concurrentqueue.h"

KOIOS_NAMESPACE_BEG

class moodycamel_queue_wrapper
{
public:
    using invocable_type = ::std::function<void()>;
    using queue_type = moodycamel::ConcurrentQueue<invocable_type>;

public:
    void enqueue(invocable_type&& func);
    ::std::optional<invocable_type> dequeue();
    bool empty() const;

private:
    queue_type m_q;
};

KOIOS_NAMESPACE_END

#endif
