// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/runtime.h"
#include "koios/timer.h"
#include "koios/event_loop.h"
#include "koios/task_scheduler.h"
#include "toolpex/exceptions.h"

#include "spdlog/spdlog.h"

#include <utility>
#include <atomic>
#include <exception>
#include <signal.h>
#include <memory>

KOIOS_NAMESPACE_BEG

namespace
{
    ::std::unique_ptr<event_loop_t> g_ts_p;

    void logging_init()
    {
        spdlog::set_level(spdlog::level::info);
    }

    void signal_handler(int signum)
    {
        static ::std::atomic_bool called{ false };       

        if (called)
        {
            koios::log_info("koios: SIGINT receive again, system will be terminate right now!");
            ::std::terminate();
        }

        called = true;
        koios::log_info("koios: SIGINT receive, cleaning");
        g_ts_p->quick_stop();
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
::std::unique_ptr<event_loop_t>&
get_task_scheduler_ptr(::std::source_location sl)
{
    if (!g_ts_p) [[unlikely]]
    {
        koios::log_error("koios: runtime has not been started! terminating...");
        ::std::terminate();
    }
    return g_ts_p;
}

/*! \brief Initialize this koios runtime.
 *  \param numthr the number of thread the runtime managed.
 */
void runtime_init(runtime_init_config cfg)
{
    logging_init();
    koios::log_info("runtime initializing");

    signal_handle_init();

    if (!cfg.m_manualy_stop)
        g_ts_p = ::std::make_unique<event_loop_t>(cfg.m_number_thread);
    else g_ts_p = ::std::make_unique<event_loop_t>(cfg.m_number_thread, manually_stop);

    auto& schr = *g_ts_p;
    for (auto& loop : cfg.m_loops)
    {
        schr.as_loop<user_event_loops>().add_loop(loop);
    }
    g_ts_p->start();
}

/*! \brief Clean the koios runtime, stop all of the resource needs `stop()` or `close()` etc.
 *  \retval EXIT_SUCCESS everything goes right, system exit normally.
 *  \retval EXIT_FAILURE something wrong.
 */
int runtime_exit()
{
    g_ts_p->quick_stop();
    return 0;
}

void runtime_reload(runtime_init_config cfg)
{
    toolpex::not_implemented();   
}

KOIOS_NAMESPACE_END
