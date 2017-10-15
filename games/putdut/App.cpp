#include "App.h"

#include "gfx/gfx.h"
#include "os/os.h"
#include "input/input.h"
#include "sound/sound.h"

#include <memory.h>

App::App()
    : currentView_(VIEW_NONE)
{
    loadResources();

    memset(&viewStates_, 0, sizeof(viewStates_));

    viewStates_[VIEW_SPLASH].enter = &App::splashEnter;
    viewStates_[VIEW_SPLASH].leave = &App::splashLeave;
    viewStates_[VIEW_SPLASH].update = &App::splashUpdate;
    viewStates_[VIEW_SPLASH].render = &App::splashRender;

    viewStates_[VIEW_MAINMENU].enter = &App::mainMenuEnter;
    viewStates_[VIEW_MAINMENU].leave = &App::mainMenuLeave;
    viewStates_[VIEW_MAINMENU].update = &App::mainMenuUpdate;
    viewStates_[VIEW_MAINMENU].render = &App::mainMenuRender;

    viewStates_[VIEW_GAME].enter = &App::gameEnter;
    viewStates_[VIEW_GAME].leave = &App::gameLeave;
    viewStates_[VIEW_GAME].update = &App::gameUpdate;
    viewStates_[VIEW_GAME].render = &App::gameRender;

    game_ = new Game();

    switchView(VIEW_GAME);
}

App::~App()
{
    delete game_;
}

void App::update()
{
    switchedView_ = false;
    if (viewStates_[currentView_].update) {
        ((*this).*viewStates_[currentView_].update)();
    }
}

void App::render()
{
    if (switchedView_)
        return;

    if (viewStates_[currentView_].render) {
        ((*this).*viewStates_[currentView_].render)();
    }
}

void App::switchView(App::View view)
{
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

int App::viewElapsedMS()
{
    uint64_t diff = os::mono() - currentViewEnterTime_;
    return (int)(diff / 1000);
}

void App::loadResources()
{
    // VIEW_SPLASH
    splashBGM_ = sound::loadOGG("data/splash.ogg", 1, false);

    // VIEW_MAINMENU
    mainMenuBGM_ = sound::loadOGG("data/mainmenu.ogg", 1, true);
    mainMenuSfxLow_ = sound::loadOGG("data/menu_low.ogg", 1, false);
    mainMenuSfxHigh_ = sound::loadOGG("data/menu_high.ogg", 1, false);
    mainMenuFont_ = gfx::loadFont("yoster");
    mainMenuLogo_ = gfx::loadPNG("data/logo.png");
}

// --------------------------------------------------------------------------------------
// Splash

#define SPLASH_FADE_IN_TIME 1000
#define SPLASH_SHOW_TIME 1000
#define SPLASH_FADE_OUT_TIME 1000
#define SPLASH_TOTAL_TIME (SPLASH_FADE_IN_TIME + SPLASH_SHOW_TIME + SPLASH_FADE_OUT_TIME)

void App::splashEnter()
{
    sound::play(splashBGM_);
}

void App::splashLeave()
{
    sound::stop(splashBGM_);
}

void App::splashUpdate()
{
    if ((viewElapsedMS() > SPLASH_TOTAL_TIME)
        || input::pressed(input::START)
        || input::pressed(input::ACCEPT)
        || input::pressed(input::CANCEL))
    {
        switchView(VIEW_MAINMENU);
    }
}

void App::splashRender()
{
    float fontSize = 0.05f * os::windowHeight();
    float opacity = 1.0f;
    uint64_t t = viewElapsedMS();
    if (t < SPLASH_FADE_IN_TIME) {
        opacity = os::clamp((float)t / SPLASH_FADE_IN_TIME, 0.0f, 1.0f);
    } else {
        t -= SPLASH_FADE_IN_TIME;
        if (t < SPLASH_SHOW_TIME) {
            opacity = 1.0;
        } else {
            t -= SPLASH_SHOW_TIME;
            opacity = 1.0f - os::clamp((float)t / SPLASH_FADE_OUT_TIME, 0.0f, 1.0f);
        }
    }
    gfx::Color textColor = { 255, 255, 255, (unsigned char)(opacity * 255.0f) };
    gfx::drawText((float)os::windowWidth() / 2, (float)os::windowHeight() / 2, "Putdut Games presents...", mainMenuFont_, fontSize, &textColor);
}

// --------------------------------------------------------------------------------------
// MainMenu

void App::mainMenuEnter()
{
    mainMenuIndex_ = 0;

    sound::play(mainMenuBGM_);
}

void App::mainMenuLeave()
{
    sound::stop(mainMenuBGM_);
}

void App::mainMenuUpdate()
{
    if(input::pressed(input::UP)) {
        sound::play(mainMenuSfxLow_);
        --mainMenuIndex_;
        if(mainMenuIndex_ < 0)
            mainMenuIndex_ = 2;
    }
    if(input::pressed(input::DOWN)) {
        sound::play(mainMenuSfxLow_);
        mainMenuIndex_ = (mainMenuIndex_ + 1) % 3;
    }

    if(input::released(input::ACCEPT)) {
        sound::play(mainMenuSfxHigh_);
        switch(mainMenuIndex_) {
            case 0: // Continue
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
    gfx::Color unselectedColor = { 255, 255, 255, 255 };
    gfx::Color selectedColor = { 46, 201, 112, 255 };
    if(input::held(input::ACCEPT)) {
        selectedColor = { 234, 77, 60, 255 };
    }
    float y = os::windowHeight() / 2.0f;
    gfx::drawText((float)os::windowWidth() / 2, y, "Continue", mainMenuFont_, menuFontSize, (mainMenuIndex_ == 0) ? &selectedColor : &unselectedColor);
    y += menuFontSize * 2.0f;
    gfx::drawText((float)os::windowWidth() / 2, y, "New Game", mainMenuFont_, menuFontSize, (mainMenuIndex_ == 1) ? &selectedColor : &unselectedColor);
    y += menuFontSize * 2.0f;
    gfx::drawText((float)os::windowWidth() / 2, y, "Quit", mainMenuFont_, menuFontSize, (mainMenuIndex_ == 2) ? &selectedColor : &unselectedColor);
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
