#define _CRT_SECURE_NO_WARNINGS

#include "os/os.h"

#include <stdio.h>

namespace os
{

void startup()
{
}

void shutdown()
{
}

void printf(const char * format, ...)
{
    int nSize = 0;
    static const int maxStackBufferSize = 512;
    char stackBuffer[maxStackBufferSize];
    char * buffer = stackBuffer;
    memset(buffer, 0, maxStackBufferSize);
    va_list args;
    va_start(args, format);
    nSize = _vsnprintf(nullptr, 0, format, args);
    if (nSize >= maxStackBufferSize) {
        buffer = (char *)malloc(nSize + 1);
        memset(buffer, 0, nSize + 1);
    }
    nSize = _vsnprintf(buffer, nSize, format, args);

    OutputDebugString(buffer);
    ::printf("%s", buffer);

    if (buffer != stackBuffer) {
        free(buffer);
    }
}

}
