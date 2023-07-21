#include "koios/runtime.h"
#include "koios/task_scheduler.h"
#include "spdlog/spdlog.h"
#include "toolpex/exceptions.h"

#include <utility>
#include <atomic>
#include <signal.h>

KOIOS_NAMESPACE_BEG

namespace
{
    ::std::unique_ptr<task_scheduler> g_ts_p;

    void logging_init()
    {
        spdlog::set_level(spdlog::level::info);
    }

    void signal_handler(int signum)
    {
        ::std::unique_ptr<task_scheduler> ts_p{};
        g_ts_p.swap(ts_p);
        spdlog::info("koios: SIGINT receive, cleaning");
    }

    void signal_handle_init()
    {
        struct ::sigaction sa{ };

        sa.sa_handler = signal_handler, 
        sa.sa_flags   = SA_RESTART,
        ::sigemptyset(&sa.sa_mask);

        if (::sigaction(SIGINT, &sa, nullptr) == -1)
        {
            throw koios::exception{};
        }
    }
}

/*! \brief Get the global task_scheduler.
 *  \return the reference of the global task_scheduler
 *  \param sl user could just igore it, its the source location of the caller.
 *
 *  \warning You has to call the `runtime_init()` function first.
 */
task_scheduler& get_task_scheduler(::std::source_location sl)
{
    if (!g_ts_p) [[unlikely]]
        throw runtime_not_working_exception{ ::std::move(sl) };
    return *g_ts_p;
}

/*! \brief Initialize this koios runtime.
 *  \param numthr the number of thread the runtime managed.
 *  \param manually_stop_type A tag which indecicate 
 *                            that the `thread_pool` or something like `task_scheduler`
 *                            needs user call `stop()` to terminate manually.
 */
void runtime_init(size_t numthr, manually_stop_type)
{
    logging_init();
    signal_handle_init();

    g_ts_p.reset(new task_scheduler{ numthr, manually_stop });
}

/*! \brief Initialize this koios runtime.
 *  \param numthr the number of thread the runtime managed.
 */
void runtime_init(size_t numthr)
{
    logging_init();
    signal_handle_init();

    g_ts_p.reset(new task_scheduler{ numthr });
}

/*! \brief Clean the koios runtime, stop all of the resource needs `stop()` or `close()` etc.
 *  \retval EXIT_SUCCESS everything goes right, system exit normally.
 *  \retval EXIT_FAILURE something wrong.
 */
int runtime_exit()
{
    g_ts_p.reset(nullptr);
    return 0;
}

/*! \brief Call `std::exchange` on the unique resource.
 *  \param other A new `task_scheduler` unique pointer.
 *  \return the old `task_scheduler` unique pointer.
 */
::std::unique_ptr<task_scheduler> 
exchange_task_scheduler(::std::unique_ptr<task_scheduler> other)
{
    return ::std::exchange(g_ts_p, ::std::move(other));
}

void runtime_reload(size_t numthr)
{
    toolpex::not_implemented();   
}

void runtime_reload(size_t numthr, manually_stop_type)
{
    toolpex::not_implemented();   
}

KOIOS_NAMESPACE_END
