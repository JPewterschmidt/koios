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
