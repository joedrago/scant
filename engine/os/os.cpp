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

bool readFile(const char *path, std::vector<unsigned char> &bytes)
{
    FILE *f = fopen(path, "rb");
    if(!f) {
        return false;
    }

    fseek(f, 0, SEEK_END);
    size_t fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    bytes.resize(fileSize);
    size_t bytesRead = fread(&bytes[0], 1, fileSize, f);
    bool success = (fileSize == bytesRead);
    fclose(f);
    return success;
}

}
