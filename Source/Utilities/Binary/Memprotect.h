/*
    Initial author: Convery
    Started: 2017-03-27
    License: MIT
*/

#pragma once
#include <cstdint>

namespace Memprotect
{
    void Protectrange(void *Address, const size_t Length, unsigned long Oldprotect);
    unsigned long Unprotectrange(void *Address, const size_t Length);
}
