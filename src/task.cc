/* koios, a c++ async runtime library.
 * copyright (c) 2024  jeremy pewterschmidt
 * 
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license
 * as published by the free software foundation; either version 2
 * of the license, or (at your option) any later version.
 * 
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 * 
 * you should have received a copy of the gnu general public license
 * along with this program; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301, usa.
 */

#include "koios/task.h"
#include "koios/task_scheduler.h"
#include "koios/runtime.h"
#include "koios/task_scheduler_concept.h"

#include <string>
#include <string_view>

KOIOS_NAMESPACE_BEG

template class koios::_task<void, run_this_async, discardable>;
template class koios::_task<void, run_this_async, non_discardable>;
template class koios::_task<bool, run_this_async, discardable>;
template class koios::_task<int, run_this_async, discardable>;
template class koios::_task<size_t, run_this_async, discardable>;
template class koios::_task<::std::string, run_this_async, discardable>;
template class koios::_task<::std::string_view, run_this_async, discardable>;

KOIOS_NAMESPACE_END
