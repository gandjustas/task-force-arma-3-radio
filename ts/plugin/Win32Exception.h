#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <limits>
#include <stdexcept>

class Win32Exception : public std::runtime_error
{
public:
    Win32Exception(DWORD error, const char* msg) noexcept;
    DWORD GetErrorCode() const;
};

void ThrowLastErrorIf(bool expression, const char* msg);
void ThrowLastErrorIf(bool expression, const char* msg, DWORD error);
