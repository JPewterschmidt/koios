#include "koios/utility.h"

namespace koios
{

static 
lazy_task<::std::chrono::system_clock::time_point> 
system_response_cost_helper()
{
    co_return ::std::chrono::system_clock::now();
}

lazy_task<::std::chrono::milliseconds> system_response_cost()
{
    const auto now = ::std::chrono::system_clock::now();
    const auto after_turnaround = co_await system_response_cost_helper();
    co_return ::std::chrono::duration_cast<::std::chrono::milliseconds>(after_turnaround - now);
}

} // namespace koios
