#include "input/input.h"

#include "os/os.h"

#include <windows.h>
#include <xinput.h>

#pragma comment (lib, "xinput9_1_0.lib")

unsigned int prevButtons_;
unsigned int currButtons_;

namespace input
{

void startup()
{
    prevButtons_ = 0;
    currButtons_ = 0;
}

void shutdown()
{
}

void update()
{
    prevButtons_ = currButtons_;
    currButtons_ = 0;

    DWORD dwResult;
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));

        dwResult = XInputGetState(i, &state);
        if (dwResult == ERROR_SUCCESS) {
            if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) {
                currButtons_ |= input::UP;
            }
            if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
                currButtons_ |= input::DOWN;
            }
            if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
                currButtons_ |= input::LEFT;
            }
            if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
                currButtons_ |= input::RIGHT;
            }
            if (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) {
                currButtons_ |= input::START;
            }
            if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
                currButtons_ |= input::ACCEPT;
            }
            if (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) {
                currButtons_ |= input::CANCEL;
            }

#if defined(_DEBUG)
            if (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) {
                currButtons_ |= input::DEBUG;
            }
#endif

            if (abs(state.Gamepad.sThumbLX) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
                if (state.Gamepad.sThumbLX < 0) {
                    currButtons_ |= input::LEFT;
                } else {
                    currButtons_ |= input::RIGHT;
                }
            }
            if (abs(state.Gamepad.sThumbLY) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
                if (state.Gamepad.sThumbLY < 0) {
                    currButtons_ |= input::DOWN;
                } else {
                    currButtons_ |= input::UP;
                }
            }
        }
    }

    if (os::windowHasFocus()) {
        if (GetAsyncKeyState(VK_UP)) {
            currButtons_ |= input::UP;
        }
        if (GetAsyncKeyState(VK_DOWN)) {
            currButtons_ |= input::DOWN;
        }
        if (GetAsyncKeyState(VK_LEFT)) {
            currButtons_ |= input::LEFT;
        }
        if (GetAsyncKeyState(VK_RIGHT)) {
            currButtons_ |= input::RIGHT;
        }
        if (GetAsyncKeyState(VK_RETURN) || GetAsyncKeyState(VK_SPACE)) {
            currButtons_ |= input::ACCEPT;
        }
        if (GetAsyncKeyState(VK_BACK)) {
            currButtons_ |= input::CANCEL;
        }
        if (GetAsyncKeyState(VK_ESCAPE)) {
            currButtons_ |= input::START;
        }
#if defined(_DEBUG)
        if (GetAsyncKeyState('D')) {
            currButtons_ |= input::DEBUG;
        }
#endif
    }
}

bool pressed(int button)
{
    return ((currButtons_ & button) != 0) && ((prevButtons_ & button) == 0);
}

bool released(int button)
{
    return ((currButtons_ & button) == 0) && ((prevButtons_ & button) != 0);
}

bool held(int button)
{
    return (currButtons_ & button) != 0;
}

}
