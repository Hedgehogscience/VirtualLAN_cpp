/*
    Initial author: Convery
    Started: 2017-4-28
    License: MIT
*/

#include "Patternscan.h"
#include "../Text/Variadicstring.h"

std::pair<size_t, size_t> Coderange;
std::pair<size_t, size_t> Datarange;

namespace
{
#ifdef _WIN32
#include <Windows.h>

    struct Rangeinitializer
    {
        Rangeinitializer()
        {
            HMODULE Module = GetModuleHandleA(NULL);
            if (!Module) return;

            PIMAGE_DOS_HEADER DOSHeader = (PIMAGE_DOS_HEADER)Module;
            PIMAGE_NT_HEADERS NTHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)Module + DOSHeader->e_lfanew);

            Coderange.first = size_t(Module) + NTHeader->OptionalHeader.BaseOfCode;
            Coderange.second = Coderange.first + NTHeader->OptionalHeader.SizeOfCode;

            Datarange.first = Coderange.second;
            Datarange.second = Datarange.first + NTHeader->OptionalHeader.SizeOfInitializedData;
        }
    };
#else
    struct Rangeinitializer
    {
        Rangeinitializer()
        {
            /* TODO(Convery): Nix stuff here. */
        }
    };
#endif

    Rangeinitializer Initializer{};
}
uint32_t Longest = 0;
namespace Patternscan
{
    // Formatting, e.g. "00 04 EB 84 ? ? 32"
    Pattern Fromstring(std::string Input)
    {
        Pattern Result;

        // Iterate through the input.
        for (auto Iterator = Input.c_str(); *Iterator; ++Iterator)
        {
            // Ignore whitespace.
            if (*Iterator == ' ') continue;

            // Check for inactive bytes.
            if (*Iterator == '?')
            {
                Result.first.append(1, '\x00');
                Result.second.append(1, '\x00');
                continue;
            }

            // Grab two bytes from the input.
            Result.first.append(1, char(strtoul(Iterator, nullptr, 16)));
            Result.second.append(1, '\x01');
            Iterator++;
        }

        return Result;
    }
    std::string Tostring(Pattern Input)
    {
        std::string Result;

        // Iterate through the input.
        for (size_t i = 0; i < Input.first.size(); ++i)
        {
            // Check for inactive bytes.
            if (Input.second[i] == '\x00')
            {
                Result.append("? ");
                continue;
            }

            // Grab the byte.
            Result.append(va("%02hhX ", Input.first[i]));
        }

        return Result;
    }

    // Base functionality for the scanner.
    size_t Find(Pattern Input, size_t Rangestart, size_t Rangeend)
    {
        uint8_t Firstbyte = Input.first[0];


        // We do not handle masks that start with an invalid byte.
        if (Input.second[0] == '\x00') return 0;

        // Iterate over the specified range.
        for (; Rangestart < Rangeend; ++Rangestart)
        {
            // Skip irrelevant bytes.
            if (*(uint8_t *)Rangestart != Firstbyte) continue;

            // Compare the data to the pattern.
            if ([](size_t Address, Pattern &Input)
            {
                for (size_t i = 1; i < Input.first.size(); ++i)
                {
                    // Skip inactive bytes.
                    if (Input.second[i] == '\x00') continue;

                    // Break on an invalid compare.
                    if (*(char *)(Address + i) != Input.first[i])
                        return false;
                }

                return true;

            }(Rangestart, Input)) return Rangestart;
        }

        // Invalid address.
        return 0;
    }
    std::vector<size_t> Findall(Pattern Input, size_t Rangestart, size_t Rangeend)
    {
        std::vector<size_t> Result;
        size_t Address = Rangestart;

        do
        {
            Address = Find(Input, Address, Rangeend);
            if (Address) Result.push_back(Address++);

        } while (Address);

        return Result;
    }

    // Built-in ranges for the application.
    size_t FindCode(Pattern Input)
    {
        return Find(Input, Coderange.first, Coderange.second);
    }
    size_t FindData(Pattern Input)
    {
        return Find(Input, Datarange.first, Datarange.second);
    }
    std::vector<size_t> FindallCode(Pattern Input)
    {
        return Findall(Input, Coderange.first, Coderange.second);
    }
    std::vector<size_t> FindallData(Pattern Input)
    {
        return Findall(Input, Datarange.first, Datarange.second);
    }
}
