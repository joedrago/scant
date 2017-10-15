#ifndef APP_H
#define APP_H

#include "gfx/gfx.h"

#include <stdint.h>

#include "Game.h"

class App
{
public:
    enum View
    {
        VIEW_NONE = 0,

        VIEW_SPLASH,
        VIEW_MAINMENU,
        VIEW_GAME,

        VIEW_COUNT
    };

    App();
    ~App();

    void update();
    void render();
    void switchView(View view);
    int viewElapsedMS();

    void loadResources();

    void splashEnter();
    void splashLeave();
    void splashUpdate();
    void splashRender();

    void mainMenuEnter();
    void mainMenuLeave();
    void mainMenuUpdate();
    void mainMenuRender();

    void gameEnter();
    void gameLeave();
    void gameUpdate();
    void gameRender();

    typedef void (App::*enterFunction)();
    typedef void (App::*leaveFunction)();
    typedef void (App::*updateFunction)();
    typedef void (App::*renderFunction)();

    struct ViewState
    {
        enterFunction enter;
        leaveFunction leave;
        updateFunction update;
        renderFunction render;
    };

protected:
    View currentView_;
    uint64_t currentViewEnterTime_;
    ViewState viewStates_[VIEW_COUNT];
    bool switchedView_;

    // Splash
    int splashBGM_;

    // MainMenu
    int mainMenuBGM_;
    int mainMenuSfxLow_;
    int mainMenuSfxHigh_;
    int mainMenuFont_;
    gfx::DrawSource mainMenuLogo_;
    int mainMenuIndex_;

    // Game
    Game *game_;
};

#endif
