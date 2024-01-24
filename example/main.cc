#include "koios/thread_pool.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"
#include "koios/task.h"
#include <chrono>
#include <iostream>
#include <cassert>
#include <fcntl.h>
#include <atomic>
#include <vector>
#include <fstream>

#include "koios/work_stealing_queue.h"
#include "koios/moodycamel_queue_wrapper.h"
#include "koios/this_task.h"
#include "koios/expected.h"
#include "koios/functional.h"

#include "toolpex/tic_toc.h"
#include "toolpex/errret_thrower.h"
#include "toolpex/unique_posix_fd.h"
#include "koios/tcp_server.h"
#include "koios/iouring_awaitables.h"
#include "koios/coroutine_mutex.h"
#include <string_view>
#include "koios/iouring_connect_aw.h"

#include "koios/unique_file_state.h"

#include <fcntl.h>
#include <sys/stat.h>

using namespace koios;
using namespace ::std::chrono_literals;
using namespace ::std::string_view_literals;

namespace
{
bool success1{};
bool success2{};

emitter_task<> dummy()
{
    co_return;
}

class loop_for_test : public user_event_loop
{
public:
    void thread_specific_preparation(const per_consumer_attr& attr) noexcept override 
    { 
        success1 = true;
    }
    void stop() noexcept override { }
    void quick_stop() noexcept override { }
    void until_done() override { }
    ::std::chrono::milliseconds max_sleep_duration(const per_consumer_attr& attr) noexcept override 
    { 
        return 1000ms;
    }

    void do_occured_nonblk() noexcept override
	{
        success2 = true;
	}
};

} // annoymous namespace

int main()
try
{
    runtime_init(4);
    get_task_scheduler().as_loop<user_event_loops>().add_loop(::std::make_unique<loop_for_test>());
    dummy().result();
    runtime_exit();

    ::std::cout << success1 << success2;
    
    return 0;
}
catch (const ::std::exception& e)
{
    ::std::cout << e.what() << ::std::endl;
    koios::runtime_exit();
}
