#include "os/os.h"

#include "engine.h"

#include <string>

static std::string windowName_ = "Scant";
static bool windowFullscreen_ = true;
static HINSTANCE hinstance_ = nullptr;
static HWND hwnd_ = nullptr;
static int windowWidth_ = 1920;
static int windowHeight_ = 1080;
static bool hasFocus_ = false;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace game
{
extern void configure();
extern void startup();
extern void update();
extern void shutdown();
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    game::configure();

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = nullptr; //LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = nullptr; //LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = "ScantWindowClass";
    wcex.hIconSm = nullptr; //LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if (!RegisterClassEx(&wcex) )
        return E_FAIL;

    DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    if (windowFullscreen_) {
        windowWidth_ = GetSystemMetrics(SM_CXSCREEN);
        windowHeight_ = GetSystemMetrics(SM_CYSCREEN);
        windowStyle = WS_POPUP;
    }

    hinstance_ = hInstance;
    RECT rc = { 0, 0, windowWidth_, windowHeight_ };
    if (!windowFullscreen_) {
        // AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    }

    hwnd_ = CreateWindow("ScantWindowClass", windowName_.c_str(), windowStyle,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
        nullptr);
    if (!hwnd_)
        return E_FAIL;
    ShowWindow(hwnd_, nCmdShow);

    if(engine::startup()) {
        game::startup();

        MSG msg = { 0 };
        while (WM_QUIT != msg.message) {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) ) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else {
                engine::begin();
                game::update();
                engine::end();
            }
        }

        game::shutdown();
        engine::shutdown();
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) {
        case WM_SETFOCUS:
            hasFocus_ = true;
            ShowCursor(FALSE);
            break;

        case WM_KILLFOCUS:
            hasFocus_ = false;
            break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

namespace os
{

void setWindowName(const char * windowName)
{
    windowName_ = windowName;
}

void setWindowFullscreen(bool fullscreen)
{
    windowFullscreen_ = fullscreen;
}

int windowWidth()
{
    return windowWidth_;
}

int windowHeight()
{
    return windowHeight_;
}

HWND windowHandle()
{
    return hwnd_;
}

bool windowHasFocus()
{
    return hasFocus_;
}

void quit()
{
    PostQuitMessage(0);
}

}
