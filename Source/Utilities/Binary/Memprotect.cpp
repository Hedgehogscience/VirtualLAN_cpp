/*
    Initial author: Convery
    Started: 2017-03-27
    License: MIT
*/

#include "Memprotect.h"

#ifdef _WIN32
#include <Windows.h>
void Memprotect::Protectrange(void *Address, const size_t Length, unsigned long Oldprotect)
{
    unsigned long Temp;
    VirtualProtect(Address, Length, Oldprotect, &Temp);
}
unsigned long Memprotect::Unprotectrange(void *Address, const size_t Length)
{
    unsigned long Oldprotect;
    VirtualProtect(Address, Length, PAGE_EXECUTE_READWRITE, &Oldprotect);
    return Oldprotect;
}

#else
#include <sys/mman.h>
void Memprotect::Protectrange(void *Address, const size_t Length, unsigned long Oldprotect)
{
    mprotect(Address, Length, Oldprotect);
}
unsigned long Memprotect::Unprotectrange(void *Address, const size_t Length)
{
    /*
        TODO(Convery):
        We need to parse /proc/self/maps to get the real access.
        Implement this when needed.
    */

    mprotect(Address, Length, (PROT_READ | PROT_WRITE | PROT_EXEC));
    return (PROT_READ | PROT_EXEC);
}
#endif
