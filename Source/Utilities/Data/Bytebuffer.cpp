/*
    Initial author: Convery
    Started: 2017-03-27
    License: MIT
*/

#include <memory>
#include <algorithm>
#include "Bytebuffer.h"

// Appends the data to the internal storage.
Bytebuffer::Bytebuffer(uint32_t Inputsize, const void *Inputdata)
{
    if (Inputdata)
    {
        Internalstorage.append((const uint8_t *)Inputdata, Inputsize);
    }
}
Bytebuffer::Bytebuffer() {};

// Internal storage access.
bool Bytebuffer::SetPosition(size_t Value)
{
    if (Value < Internalstorage.size())
        Storageiterator = Value;
    return Storageiterator == Value;
}
size_t Bytebuffer::Position()
{
    return Storageiterator;
}
uint8_t Bytebuffer::Peek()
{
    uint8_t Byte = BB_MAX;

    if (RawRead(sizeof(uint8_t), &Byte))
    {
        // Rewind the buffer.
        SetPosition(Position() - 1);
    }

    return Byte;
}
size_t Bytebuffer::Size()
{
    return Internalstorage.size();
}
void Bytebuffer::Rewind()
{
    SetPosition(0);
}
void Bytebuffer::Clear()
{
    Storageiterator = 0;
    Internalstorage.clear();
    Internalstorage.shrink_to_fit();
}
const uint8_t *Bytebuffer::Data()
{
    return Internalstorage.data();
}

// Core functionality.
bool Bytebuffer::ReadDatatype(Bytebuffertype Type)
{
    if (Type == Peek()) return SetPosition(Position() + 1);
    else return false;
}
bool Bytebuffer::WriteDatatype(Bytebuffertype Type)
{
    return RawWrite(sizeof(Bytebuffertype), &Type);
}
bool Bytebuffer::RawRead(size_t Readcount, void *Buffer)
{
    // Rangecheck, we do not handle truncated reads.
    if ((Storageiterator + Readcount) > Internalstorage.size())
        return false;

    // Copy the data into the new buffer if valid.
    if (Buffer) std::memcpy(Buffer, Internalstorage.data() + Storageiterator, Readcount);

    // Increment the iterator and return.
    Storageiterator += Readcount;
    return true;
}
bool Bytebuffer::RawWrite(size_t Writecount, const void *Buffer)
{
    // If there's no valid buffer, just pretend to write.
    if (!Buffer) return SetPosition(Position() + Writecount);

    // At the end of the buffer, append.
    if (Storageiterator == Internalstorage.size())
    {
        Internalstorage.append((const uint8_t *)Buffer, Writecount);
        Storageiterator += Writecount;
        return true;
    }

    // If the buffer is large enough, just replace the data.
    if ((Storageiterator + Writecount) < Internalstorage.size())
    {
        Internalstorage.replace(Storageiterator, Writecount, (const uint8_t *)Buffer, Writecount);
        Storageiterator += Writecount;
        return true;
    }

    // Else we split the data, replace and append.
    size_t Remaining = Internalstorage.size() - Storageiterator;
    return RawWrite(Remaining, Buffer) && RawWrite(Writecount - Remaining, (const uint8_t *)Buffer + Remaining);
}

// Simple datatypes.
#define SIMPLE_TEMPLATE(Type, Enum)                                         \
template <> bool Bytebuffer::Read(Type &Buffer, bool Typechecked)           \
{                                                                           \
    if(!Typechecked || ReadDatatype(Enum))                                  \
        return RawRead(sizeof(Buffer), &Buffer);                            \
    else return false;                                                      \
}                                                                           \
template <> Type Bytebuffer::Read(bool Typechecked)                         \
{                                                                           \
    Type Result{};                                                          \
    Read(Result, Typechecked);                                              \
    return Result;                                                          \
}                                                                           \
template <> bool Bytebuffer::Write(const Type Buffer, bool Typechecked)     \
{                                                                           \
    if(Typechecked) WriteDatatype(Enum);                                    \
    return RawWrite(sizeof(Buffer), &Buffer);                               \
}                                                                           \

SIMPLE_TEMPLATE(bool, BB_BOOL);
SIMPLE_TEMPLATE(char, BB_SINT8);
SIMPLE_TEMPLATE(int8_t, BB_SINT8);
SIMPLE_TEMPLATE(uint8_t, BB_UINT8);
SIMPLE_TEMPLATE(wchar_t, BB_WCHAR);
SIMPLE_TEMPLATE(int16_t, BB_SINT16);
SIMPLE_TEMPLATE(uint16_t, BB_UINT16);
SIMPLE_TEMPLATE(int32_t, BB_SINT32);
SIMPLE_TEMPLATE(uint32_t, BB_UINT32);
SIMPLE_TEMPLATE(int64_t, BB_SINT8);
SIMPLE_TEMPLATE(uint64_t, BB_UINT8);
SIMPLE_TEMPLATE(float, BB_FLOAT32);
SIMPLE_TEMPLATE(double, BB_FLOAT64);

// Advanced datatypes.
std::string Bytebuffer::ReadBlob(bool Typechecked)
{
    std::string Result{};
    ReadBlob(Result, Typechecked);
    return Result;
}
std::string Bytebuffer::ReadString(bool Typechecked)
{
    std::string Result{};
    ReadString(Result, Typechecked);
    return Result;
}
std::wstring Bytebuffer::ReadWString(bool Typechecked)
{
    std::wstring Result{};
    ReadWString(Result, Typechecked);
    return Result;
}
bool Bytebuffer::ReadBlob(std::string &Buffer, bool Typechecked)
{
    if (!Typechecked || ReadDatatype(BB_BLOB))
    {
        uint32_t Bloblength = Read<uint32_t>();
        auto Localbuffer = std::make_unique<char[]>(Bloblength);

        if (RawRead(Bloblength, Localbuffer.get()))
        {
            Buffer.append(Localbuffer.get(), Bloblength);
            return true;
        }
    }
    return false;
}
bool Bytebuffer::WriteBlob(std::string *Buffer, bool Typechecked)
{
    if (Typechecked) WriteDatatype(BB_BLOB);
    return Write(uint32_t(Buffer->size())) && RawWrite(Buffer->size(), Buffer->data());
}
bool Bytebuffer::ReadString(std::string &Buffer, bool Typechecked)
{
    if (!Typechecked || ReadDatatype(BB_STRING_ASCII))
    {
        size_t Stringlength = std::strlen((const char *)Internalstorage.data() + Storageiterator) + 1;
        Buffer.append((const char *)Internalstorage.data() + Storageiterator);
        return SetPosition(Position() + Stringlength);
    }
    return false;
}
bool Bytebuffer::WriteString(std::string *Buffer, bool Typechecked)
{
    if (Typechecked) WriteDatatype(BB_STRING_ASCII);
    return RawWrite(Buffer->size() + 1, Buffer->c_str());
}
bool Bytebuffer::ReadWString(std::wstring &Buffer, bool Typechecked)
{
    if (!Typechecked || ReadDatatype(BB_STRING_UNICODE))
    {
        size_t Stringlength = std::wcslen((wchar_t *)(Internalstorage.data() + Storageiterator));
        Buffer.append((wchar_t *)(Internalstorage.data() + Storageiterator));
        return SetPosition(Position() + Stringlength * sizeof(wchar_t) + 1);
    }
    return false;
}
bool Bytebuffer::WriteWString(std::wstring *Buffer, bool Typechecked)
{
    if (Typechecked) WriteDatatype(BB_STRING_UNICODE);
    return RawWrite((Buffer->size() + 1) * sizeof(wchar_t), Buffer->c_str());
}

// Array datatypes.
#define ARRAY_TEMPLATE(Type, Enum)                                          \
template <> bool Bytebuffer::ReadArray(std::vector<Type> &Data)             \
{                                                                           \
    uint8_t Storedtype = Read<uint8_t>(false);                              \
    if (Storedtype != Enum + 100) return false;                             \
                                                                            \
    uint32_t Storedcount = Read<uint32_t>();                                \
    for (; Storedcount; --Storedcount)                                      \
        Data.push_back({ Read<Type>(false) });                              \
    return true;                                                            \
}                                                                           \
template <> bool Bytebuffer::WriteArray(std::vector<Type> *Data)            \
{                                                                           \
    Write(uint8_t(Enum + 100), false);                                      \
    Write(uint32_t(Data->size()));                                          \
                                                                            \
    for (auto &Item : *Data) Write(Item, false);                            \
    return true;                                                            \
}                                                                           \

//ARRAY_TEMPLATE(bool, BB_BOOL);
ARRAY_TEMPLATE(char, BB_SINT8);
ARRAY_TEMPLATE(int8_t, BB_SINT8);
ARRAY_TEMPLATE(uint8_t, BB_UINT8);
ARRAY_TEMPLATE(wchar_t, BB_WCHAR);
ARRAY_TEMPLATE(int16_t, BB_SINT16);
ARRAY_TEMPLATE(uint16_t, BB_UINT16);
ARRAY_TEMPLATE(int32_t, BB_SINT32);
ARRAY_TEMPLATE(uint32_t, BB_UINT32);
ARRAY_TEMPLATE(int64_t, BB_SINT8);
ARRAY_TEMPLATE(uint64_t, BB_UINT8);
ARRAY_TEMPLATE(float, BB_FLOAT32);
ARRAY_TEMPLATE(double, BB_FLOAT64);

// Ease serialization by appending buffers.
Bytebuffer &Bytebuffer::operator = (Bytebuffer &Right)
{
    this->Clear();
    return *this + Right;
}
Bytebuffer &Bytebuffer::operator + (Bytebuffer Right)
{
    this->RawWrite(Right.Size(), Right.Data());
    return *this;
}
Bytebuffer &Bytebuffer::operator += (Bytebuffer Right)
{
    this->RawWrite(Right.Size(), Right.Data());
    return *this;
}
