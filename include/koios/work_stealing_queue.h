#ifndef KOIOS_WORK_STEALING_QUEUE_H
#define KOIOS_WORK_STEALING_QUEUE_H

#include "koios/macros.h"
#include "koios/queue_concepts.h"
#include "koios/moodycamel_queue_wrapper.h"

KOIOS_NAMESPACE_BEG

template<typename T, queue_concept LockFreeQueue>
class work_stealing_queue
{
public:
private:
    
};

KOIOS_NAMESPACE_END

#endif
