// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/lite_task.h"
#include "koios/task_scheduler.h"
#include "koios/runtime.h"

template class koios::_lite_task<void, koios::discardable, ::std::suspend_always>::_type;
template class koios::_lite_task<void, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<void, koios::non_discardable, koios::eager_aw>::_type;
template class koios::_lite_task<bool, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<int, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<size_t, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<::std::string, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<::std::string_view, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<::std::error_code, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<uint8_t, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<uint32_t, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<::std::byte*, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<const ::std::byte*, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<char*, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<const char*, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<void*, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<const void*, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<::std::span<::std::byte>, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<::std::span<const ::std::byte>, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<::std::span<char>, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<::std::span<const char>, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<::std::span<uint8_t>, koios::discardable, koios::eager_aw>::_type;
template class koios::_lite_task<::std::span<const uint8_t>, koios::discardable, koios::eager_aw>::_type;

