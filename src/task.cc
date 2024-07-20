// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/task.h"
#include "koios/task_scheduler.h"
#include "koios/runtime.h"

KOIOS_NAMESPACE_BEG

template class koios::_task<void, discardable, lazy_aw>::_type;
template class koios::_task<void, discardable, eager_aw>::_type;
template class koios::_task<void, non_discardable, eager_aw>::_type;
template class koios::_task<bool, discardable, eager_aw>::_type;
template class koios::_task<int, discardable, eager_aw>::_type;
template class koios::_task<size_t, discardable, eager_aw>::_type;
template class koios::_task<::std::string, discardable, eager_aw>::_type;
template class koios::_task<::std::string_view, discardable, eager_aw>::_type;
template class koios::_task<::std::error_code, discardable, eager_aw>::_type;
template class koios::_task<uint8_t, discardable, eager_aw>::_type;
template class koios::_task<uint32_t, discardable, eager_aw>::_type;
template class koios::_task<::std::byte*, discardable, eager_aw>::_type;
template class koios::_task<const ::std::byte*, discardable, eager_aw>::_type;
template class koios::_task<char*, discardable, eager_aw>::_type;
template class koios::_task<const char*, discardable, eager_aw>::_type;
template class koios::_task<void*, discardable, eager_aw>::_type;
template class koios::_task<const void*, discardable, eager_aw>::_type;
template class koios::_task<::std::span<::std::byte>, discardable, eager_aw>::_type;
template class koios::_task<::std::span<const ::std::byte>, discardable, eager_aw>::_type;
template class koios::_task<::std::span<char>, discardable, eager_aw>::_type;
template class koios::_task<::std::span<const char>, discardable, eager_aw>::_type;
template class koios::_task<::std::span<uint8_t>, discardable, eager_aw>::_type;
template class koios::_task<::std::span<const uint8_t>, discardable, eager_aw>::_type;

KOIOS_NAMESPACE_END
