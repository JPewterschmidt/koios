#ifndef KOIOS_THREAD_POOL_H
#define KOIOS_THREAD_POOL_H

#include <functional>

#include "koios/macros.h"

KOIOS_NAMESPACE_BEG

class thread_pool
{
public:
    explicit thread_pool(size_t numthr);
    void stop();
    void quick_stop();
    auto size() const noexcept { return m_numthr; }
    
    void enqueue(::std::function<void()> func);
    
private:
    size_t m_numthr;
};

KOIOS_NAMESPACE_END

#endif
