#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"
#include <chrono>
#include <iostream>
#include <cassert>

#include "koios/work_stealing_queue.h"
#include "koios/moodycamel_queue_wrapper.h"

using namespace koios;
using namespace ::std::chrono_literals;

int main()
{
    work_stealing_queue<moodycamel_queue_wrapper> q;

    
    return 0;
}
