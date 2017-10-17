#ifndef APP_H
#define APP_H

#include "gfx/gfx.h"

#include <stdint.h>

#include "Game.h"
#include "Cutscene.h"

class App
{
public:
    enum View
    {
        VIEW_NONE = 0,

        VIEW_CUTSCENE,
        VIEW_MAINMENU,
        VIEW_GAME,

        VIEW_COUNT
    };

    App();
    ~App();

    void update();
    void render();
    void switchView(View view);
    void playCutscene(const char *sceneName, View after);
    int viewElapsedMS();

    void loadResources();

    void cutsceneEnter();
    void cutsceneLeave();
    void cutsceneUpdate();
    void cutsceneRender();

    void mainMenuEnter();
    void mainMenuLeave();
    void mainMenuUpdate();
    void mainMenuRender();

    void gameEnter();
    void gameLeave();
    void gameUpdate();
    void gameRender();

    typedef void (App::* enterFunction)();
    typedef void (App::* leaveFunction)();
    typedef void (App::* updateFunction)();
    typedef void (App::* renderFunction)();

    struct ViewState
    {
        enterFunction enter;
        leaveFunction leave;
        updateFunction update;
        renderFunction render;
    };

    gfx::DrawSource & logo() { return mainMenuLogo_; }

protected:
    View currentView_;
    uint64_t currentViewEnterTime_;
    ViewState viewStates_[VIEW_COUNT];
    bool switchedView_;

    // Cutscene
    Cutscene cutscene_;
    View afterCutscene_;

    // MainMenu
    int mainMenuBGM_;
    int mainMenuSfxLow_;
    int mainMenuSfxHigh_;
    int mainMenuFont_;
    gfx::DrawSource mainMenuLogo_;
    int mainMenuIndex_;
    int gotoIndex_;

    // Game
    Game * game_;
};

#endif // ifndef APP_H
