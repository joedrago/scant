#include <windows.h>

extern void gameMain();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    gameMain();
    return 0;
}
