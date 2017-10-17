#include "App.h"

#include "gfx/gfx.h"
#include "os/os.h"
#include "input/input.h"
#include "sound/sound.h"

#include "version.h"

#include <memory.h>

App::App()
    : currentView_(VIEW_NONE)
{
    loadResources();

    memset(&viewStates_, 0, sizeof(viewStates_));

    viewStates_[VIEW_CUTSCENE].enter = &App::cutsceneEnter;
    viewStates_[VIEW_CUTSCENE].leave = &App::cutsceneLeave;
    viewStates_[VIEW_CUTSCENE].update = &App::cutsceneUpdate;
    viewStates_[VIEW_CUTSCENE].render = &App::cutsceneRender;

    viewStates_[VIEW_MAINMENU].enter = &App::mainMenuEnter;
    viewStates_[VIEW_MAINMENU].leave = &App::mainMenuLeave;
    viewStates_[VIEW_MAINMENU].update = &App::mainMenuUpdate;
    viewStates_[VIEW_MAINMENU].render = &App::mainMenuRender;

    viewStates_[VIEW_GAME].enter = &App::gameEnter;
    viewStates_[VIEW_GAME].leave = &App::gameLeave;
    viewStates_[VIEW_GAME].update = &App::gameUpdate;
    viewStates_[VIEW_GAME].render = &App::gameRender;

    game_ = new Game(this);
    gotoIndex_ = game_->highestLevelReached();

#if _DEBUG
    // switchView(VIEW_MAINMENU);
    playCutscene("intro", VIEW_MAINMENU);
#else
    playCutscene("intro", VIEW_MAINMENU);
    // switchView(VIEW_SPLASH);
#endif
}

App::~App()
{
    delete game_;
}

void App::update()
{
    do {
        switchedView_ = false;
        if (viewStates_[currentView_].update) {
            ((*this).*viewStates_[currentView_].update)();
        }
    } while (switchedView_);
}

void App::render()
{
    if (viewStates_[currentView_].render) {
        ((*this).*viewStates_[currentView_].render)();
    }
}

void App::switchView(App::View view)
{
    input::update(); // Hack to stop pressed() from spilling into the next view

    if (viewStates_[currentView_].leave) {
        ((*this).*viewStates_[currentView_].leave)();
    }
    currentView_ = view;
    if (viewStates_[currentView_].enter) {
        ((*this).*viewStates_[currentView_].enter)();
    }
    switchedView_ = true;
    currentViewEnterTime_ = os::mono();
}

void App::playCutscene(const char *sceneName, View after)
{
    cutscene_.startScene(sceneName);
    afterCutscene_ = after;
    switchView(VIEW_CUTSCENE);
}

int App::viewElapsedMS()
{
    uint64_t diff = os::mono() - currentViewEnterTime_;
    return (int)(diff / 1000);
}

void App::loadResources()
{
    // VIEW_CUTSCENE
    // splashBGM_ = sound::loadOGG("data/splash.ogg", 1, false);

    // VIEW_MAINMENU
    mainMenuBGM_ = sound::loadOGG("data/mainmenu.ogg", 1, true);
    mainMenuSfxLow_ = sound::loadOGG("data/menu_low.ogg", 1, false);
    mainMenuSfxHigh_ = sound::loadOGG("data/menu_high.ogg", 1, false);
    mainMenuFont_ = gfx::loadFont("yoster");
    mainMenuLogo_ = gfx::loadPNG("data/logo.png");

    cutscene_.loadScenes("csart", "data/scenes.json", mainMenuFont_);
}

// --------------------------------------------------------------------------------------
// Cutscene

void App::cutsceneEnter()
{
}

void App::cutsceneLeave()
{
}

void App::cutsceneUpdate()
{
    if(!cutscene_.update()) {
        switchView(afterCutscene_);
    }
}

void App::cutsceneRender()
{
    cutscene_.render();
}

// --------------------------------------------------------------------------------------
// MainMenu

void App::mainMenuEnter()
{
    mainMenuIndex_ = 0;
    gotoIndex_ = game_->highestLevelReached();

    sound::play(mainMenuBGM_);
}

void App::mainMenuLeave()
{
    sound::stop(mainMenuBGM_);
}

void App::mainMenuUpdate()
{
    if (viewElapsedMS() < 250)
        return;

    if (input::pressed(input::UP)) {
        sound::play(mainMenuSfxLow_);
        --mainMenuIndex_;
        if (mainMenuIndex_ < 0)
            mainMenuIndex_ = 2;
    }

    if (mainMenuIndex_ == 1) {
        if (input::pressed(input::LEFT)) {
            --gotoIndex_;
            if (gotoIndex_ < 0)
                gotoIndex_ = game_->highestLevelReached();
        }
        if (input::pressed(input::RIGHT)) {
            ++gotoIndex_;
            if (gotoIndex_ > game_->highestLevelReached())
                gotoIndex_ = 0;
        }
    }

    if (input::pressed(input::DOWN)) {
        sound::play(mainMenuSfxLow_);
        mainMenuIndex_ = (mainMenuIndex_ + 1) % 3;
    }

    if (input::pressed(input::ACCEPT)) {
        sound::play(mainMenuSfxHigh_);
        switch (mainMenuIndex_) {
            case 0: // Continue
                switchView(VIEW_GAME);
                break;
            case 1: // Go
                game_->switchLevel(gotoIndex_);
                switchView(VIEW_GAME);
                break;
            case 2: // Quit
                os::quit();
                break;
        }
    }
}

void App::mainMenuRender()
{
    float logoWidth = 0.8f * os::windowWidth();
    float logoHeight = logoWidth / mainMenuLogo_.w * mainMenuLogo_.h;

    gfx::draw(os::windowWidth() / 2.0f, os::windowHeight() * 0.05f, logoWidth, logoHeight, &mainMenuLogo_, nullptr, 0.5f, 0.0f);

    float menuFontSize = 0.05f * os::windowHeight();
    gfx::Color unselectedColor = { 128, 128, 128, 255 };
    gfx::Color selectedColor = { 46, 201, 112, 255 };
    if (input::held(input::ACCEPT)) {
        selectedColor = { 234, 77, 60, 255 };
    }
    float y = os::windowHeight() / 2.0f;
    gfx::drawText(os::winWf() / 2, y, "Continue", mainMenuFont_, menuFontSize, (mainMenuIndex_ == 0) ? &selectedColor : &unselectedColor);
    y += menuFontSize * 2.0f;

    char gotoString[1024];
    Level * level =  game_->getLevel(gotoIndex_);
    sprintf(gotoString, "< Go: %d - \"%s\"", gotoIndex_ + 1, level->title_.c_str());

    gfx::drawText(os::winWf() / 2, y, gotoString, mainMenuFont_, menuFontSize, (mainMenuIndex_ == 1) ? &selectedColor : &unselectedColor);
    y += menuFontSize * 2.0f;
    gfx::drawText(os::winWf() / 2, y, "Quit", mainMenuFont_, menuFontSize, (mainMenuIndex_ == 2) ? &selectedColor : &unselectedColor);

    {
        float versionFontSize = 0.025f * os::windowHeight();
        float versionMargin = versionFontSize / 5.0f;
        gfx::Color versionTextColor = { 64, 64, 64, 255 };
        gfx::drawText(versionMargin, os::winHf() - versionMargin, VERSION_STRING, mainMenuFont_, versionFontSize, &versionTextColor, 0.0f, 1.0f);
    }
}

// --------------------------------------------------------------------------------------
// Game

void App::gameEnter()
{
    game_->enter();
}

void App::gameLeave()
{
    game_->leave();
}

void App::gameUpdate()
{
    game_->update();
}

void App::gameRender()
{
    game_->render();
}
