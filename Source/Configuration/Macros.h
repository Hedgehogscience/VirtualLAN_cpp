/*
    Initial author: Convery
    Started: 2017-03-27
    License: MIT
*/

#pragma once

// Debug information.
#ifdef NDEBUG
#define PrintFunction()
#define DebugPrint(string)
#else
#define PrintFunction() LogPrefix(__FUNCTION__, "Call to")
#define DebugPrint(string) LogPrefix(string, "  Debug")
#endif

// General information.
#define InfoPrint(string) LogPrefix(string, "  Info")
#define VAPrint(format, ...) Log(va(format, __VA_ARGS__))
