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
