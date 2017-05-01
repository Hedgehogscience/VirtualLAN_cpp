/*
    Initial author: Convery
    Started: 2017-03-27
    License: MIT
*/

#pragma once
#include <cstdarg>
#include <string>
#include <memory>

inline std::string va(const char *Format, ...)
{
    auto Resultbuffer = std::make_unique<char[]>(2049);
    std::va_list Varlist;

    // Create a new string from the arguments and truncate as needed.
    va_start(Varlist, Format);
    std::vsnprintf(Resultbuffer.get(), 2048, Format, Varlist);
    va_end(Varlist);

    return std::string(Resultbuffer.get());
}
