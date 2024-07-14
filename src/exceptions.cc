// This file is part of Koios
// https://github.com/JPewterschmidt/koios
//
// Copyleft 2023 - 2024, ShiXin Wang. All wrongs reserved.

#include "koios/exceptions.h"
#include "spdlog/spdlog.h"

KOIOS_NAMESPACE_BEG

void exception::log() const noexcept
{
    log_info(what());
}

void log_info(::std::string msg)
{
    spdlog::info(msg);
}

void log_error(::std::string msg)
{
    spdlog::error(msg);
}

void log_debug(::std::string msg)
{
    spdlog::debug(msg);
}

KOIOS_NAMESPACE_END
