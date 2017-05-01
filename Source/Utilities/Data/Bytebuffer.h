/*
    Initial author: Convery
    Started: 2017-03-27
    License: MIT
*/

#pragma once
#include <string>
#include <vector>

// The types of data handled by the buffer.
enum Bytebuffertype : uint8_t
{
    BB_NONE = 0,
    BB_BOOL = 1,
    BB_SINT8 = 2,
    BB_UINT8 = 3,
    BB_WCHAR = 4,
    BB_SINT16 = 5,
    BB_UINT16 = 6,
    BB_SINT32 = 7,
    BB_UINT32 = 8,
    BB_SINT64 = 9,
    BB_UINT64 = 10,
    BB_FLOAT32 = 11,
    BB_FLOAT64 = 12,
    BB_STRING_ASCII = 13,
    BB_STRING_UNICODE = 14,
    BB_BLOB = 15,
    BB_MAX
};

class Bytebuffer
{
    // Internal state.
    std::basic_string<uint8_t> Internalstorage{};
    size_t Storageiterator{};

public:
    // Appends the data to the internal storage.
    Bytebuffer(uint32_t Inputsize, const void *Inputdata);
    Bytebuffer();

    // Internal storage access.
    bool SetPosition(size_t Value);     // Set the iterator position.
    size_t Position();                  // Iterator position.
    uint8_t Peek();                     // Read a byte ahead, BB_MAX on error.
    size_t Size();                      // Buffer length.
    void Rewind();                      // SetPosition(0);
    void Clear();                       // Deallocates all data in the buffer.
    const uint8_t *Data();              // Raw access to the internal storage.

    // Core functionality.
    bool ReadDatatype(Bytebuffertype Type);
    bool WriteDatatype(Bytebuffertype Type);
    bool RawRead(size_t Readcount, void *Buffer = nullptr);
    bool RawWrite(size_t Writecount, const void *Buffer = nullptr);

    // Simple datatypes.
    template <typename Type> Type Read(bool Typechecked = true);
    template <typename Type> bool Read(Type &Buffer, bool Typechecked = true);
    template <typename Type> bool Write(const Type Value, bool Typechecked = true);

    // Advanced datatypes.
    std::string ReadBlob(bool Typechecked = true);
    std::string ReadString(bool Typechecked = true);
    std::wstring ReadWString(bool Typechecked = true);
    bool ReadBlob(std::string &Buffer, bool Typechecked = true);
    bool WriteBlob(std::string *Buffer, bool Typechecked = true);
    bool ReadString(std::string &Buffer, bool Typechecked = true);
    bool WriteString(std::string *Buffer, bool Typechecked = true);
    bool ReadWString(std::wstring &Buffer, bool Typechecked = true);
    bool WriteWString(std::wstring *Buffer, bool Typechecked = true);
    template <typename Type> bool ReadArray(std::vector<Type> &Data);
    template <typename Type> bool WriteArray(std::vector<Type> *Data);

    // Ease serialization by appending buffers.
    Bytebuffer &operator = (Bytebuffer&);
    Bytebuffer &operator + (Bytebuffer);
    Bytebuffer &operator += (Bytebuffer);
};
