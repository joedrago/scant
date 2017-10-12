#ifndef OS_OS_H
#define OS_OS_H

#include <windows.h>

namespace os
{

void startup();
void shutdown();

void printf(const char *format, ...);

void setWindowName(const char * windowName);
void setWindowFullscreen(bool fullscreen);
int windowWidth();
int windowHeight();
HWND windowHandle();

}

#endif
