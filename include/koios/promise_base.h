// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#ifndef TASK_PROMISE_BASE_H
#define TASK_PROMISE_BASE_H

#include "koios/macros.h"
#include "koios/local_thread_scheduler.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_wrapper.h"
#include "koios/task_on_the_fly.h"
#include "koios/exceptions.h" // logging
#include <sstream>            // logging
 
KOIOS_NAMESPACE_BEG

/*! \brief the alias of `::std::suspend_never`
 *
 *  NEW: 
 *  As the standard guaranteed: coroutine state will be destroyed right after the final suspend point.
 *  that means there's no need to call `destroy()` manually. Using `suspend_never` as the final SP
 *  meets the requirement of this project.
 *  For other coroutine runtime or future development, you can re design a AW to delay the destroy 
 *  of a coroutine.
 *  
 *  OLD: 
 *  Coroutine which has been suspended by this awaitable class will be `destroy()`.
 *  Often used as awaitable object for `final_suspend()`.
 *  Will take ownership of the task's handler at the end of the `task` lifetime and `destroy()` it. 
 *  There are two more classes that can hold handler ownership.
 *  
 *  \see `task`
 *  \see `task_on_the_fly`
 */
using destroy_aw = ::std::suspend_never;

using eager_aw = ::std::suspend_never;
using lazy_aw = ::std::suspend_always;

/*! \brief life-cycle control class of a task
 *  \tparam FinalSuspendAwaitable Class which contains the methods
 *                                when coroutine get into the final suspend phase.
 */
template<
    typename InitialSuspendAwaitable = eager_aw, 
    typename FinalSuspendAwaitable = destroy_aw>
class promise_base
{
public:
    constexpr InitialSuspendAwaitable initial_suspend() const noexcept
        { return {}; }
    constexpr FinalSuspendAwaitable final_suspend() const noexcept
        { return {}; }
};

KOIOS_NAMESPACE_END

#endif
