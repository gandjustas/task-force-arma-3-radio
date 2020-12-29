#include <Windows.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <limits>
#include <stdexcept>

using namespace std;

string FormatErrorMessage(DWORD error, const char* msg)
{
    static const int BUFFERLENGTH = 1024;
    char buf[BUFFERLENGTH];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, buf,
        BUFFERLENGTH - 1, 0);
    string r{ msg };
    r += "   (";
    r += buf;
    r += ")";
    
    return r;
}

class Win32Exception : public runtime_error
{
private:
    DWORD m_error;
public:
    Win32Exception(DWORD error, const char* msg) noexcept;

    DWORD GetErrorCode() const;
};

Win32Exception::Win32Exception(DWORD error, const char* msg) noexcept
    : runtime_error(FormatErrorMessage(error, msg)), m_error(error) { }
DWORD Win32Exception::GetErrorCode() const { return m_error; }


void ThrowLastErrorIf(bool expression, const char* msg, DWORD error)
{
    if (expression) {
        throw Win32Exception(error, msg);
    }
}

void ThrowLastErrorIf(bool expression, const char* msg)
{
    ThrowLastErrorIf(expression, msg, ::GetLastError());
}