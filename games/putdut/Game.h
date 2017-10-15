#ifndef GAME_H
#define GAME_H

#include "gfx/gfx.h"

#include <stdint.h>

class Game
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

    Game();
    ~Game();

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

    typedef void (Game::*enterFunction)();
    typedef void (Game::*leaveFunction)();
    typedef void (Game::*updateFunction)();
    typedef void (Game::*renderFunction)();

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
    int mainMenuFont_;
    gfx::DrawSource mainMenuLogo_;
    int mainMenuIndex_;
};

#endif
