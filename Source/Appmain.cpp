/*
    Initial author: Convery
    Started: 2017-03-27
    License: MIT
    
    Not maintained anymore.
*/

#include "StdInclude.h"

// Clear the logfile on startup so we only save this session.
namespace { struct Deletelog { Deletelog() { Clearlogfile(); } }; static Deletelog Deleted{}; }

// Ayria plugin exports.
extern "C"
{
    EXPORT_ATTR void __cdecl onInitializationStart(bool Reserved)
    {
        static bool Initialized = false;
        if (Initialized) return;
        else Initialized = true;

        /*
            Usage:
            ----------------------------------------------------------------------
            This callback is called at the games entrypoint, which means that all
            other libraries are initialized. Your plugin should make all the .text
            segment modifications from this callback. Also your initialization.
        */
    }
    EXPORT_ATTR void __cdecl onInitializationDone(bool Reserved)
    {
        static bool Initialized = false;
        if (Initialized) return;
        else Initialized = true;

        /*
            Usage:
            ----------------------------------------------------------------------
            This callback is called when another module notifies the bootstrapper
            that the game is initialized, or at most 3 seconds after startup. Your
            plugin should do all its .data segment modifications from this func.
        */
    }
    EXPORT_ATTR void __cdecl onMessage(uint32_t MessageID, uint32_t Messagesize, const void *Messagedata)
    {
        /*
            Usage:
            ----------------------------------------------------------------------
            This callback is called when a plugin calls the bootstrapper export to
            broadcast a message to all plugins. The messageID should be unique to
            your plugin so there's no confusion. The data is generally formatted
            as a ByteBuffer structure, but it's not guaranteed to. Use with care.
        */

        // MessageID is a FNV1a_32 hash of a string.
        switch (MessageID)
        {
            case Compiletimehash::FNV1a_32(MODULENAME "_Default"):
            default: break;
        }
    }
}

// Default entrypoint for windows.
#ifdef _WIN32
#include <Windows.h>
BOOLEAN WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved)
{
    switch ( nReason )
    {
        case DLL_PROCESS_ATTACH:
        {
            // Rather not handle all thread updates.
            DisableThreadLibraryCalls(hDllHandle);
            break;
        }
    }

    return TRUE;
}
#endif
