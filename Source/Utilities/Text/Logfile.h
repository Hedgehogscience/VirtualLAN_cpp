/*
    Initial author: Convery
    Started: 2017-03-27
    License: MIT
*/

#pragma once
#include <cstdio>
#include <ctime>
#include <mutex>
#include "Variadicstring.h"

// Configuration defines.
#if !defined(LOGFILEDIR) || !defined(MODULENAME)
#define LOGFILEDIR "./"
#define MODULENAME "plugin"
#endif

// Full path to our logfile.
constexpr const char *Logfilepath = LOGFILEDIR MODULENAME ".log";
static std::mutex Threadguard;

// Write a string to the logfile.
inline void Log(const char *Message)
{
    // Prevent multiple threads from messing the log up.
    Threadguard.lock();
    {
        // Append to the logfile.
        auto Filehandle = std::fopen(Logfilepath, "a");
        if (Filehandle)
        {
            std::fputs(Message, Filehandle);
            std::fputs("\n", Filehandle);
            std::fclose(Filehandle);
        }
    }
    Threadguard.unlock();

    // Duplicate the output to stdout.
#ifdef DEBUGTOSTREAM
    std::fputs(Message, stdout);
    std::fputs("\n", stdout);
#endif
}
inline void Log(std::string Message)
{
    Log(Message.c_str());
}

// Write with a prefix.
inline void LogPrefix(const char *Message, const char *Prefix)
{
    return Log(va("[%-8s] %s", Prefix, Message));
}
inline void LogTimestamp(const char *Message)
{
    auto Now = std::time(NULL);
    char Buffer[80]{};

    std::strftime(Buffer, 80, "%H:%M:%S", std::localtime(&Now));
    return LogPrefix(Message, Buffer);
}

// Delete the log and create a new.
inline void Clearlogfile()
{
    std::remove(Logfilepath);
    LogTimestamp("Starting up..");
}
