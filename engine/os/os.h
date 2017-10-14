#ifndef OS_OS_H
#define OS_OS_H

#include <windows.h>
#include <string>
#include <vector>

namespace os
{

void startup();
void shutdown();

void quit();

void printf(const char * format, ...);

void setWindowName(const char * windowName);
void setWindowFullscreen(bool fullscreen);
int windowWidth();
int windowHeight();
HWND windowHandle();

bool readFile(const char *path, std::vector<unsigned char> &bytes);
bool readFile(const char *path, std::string &s);

}

#endif
