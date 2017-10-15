#include "Game.h"

#include "gfx/gfx.h"
#include "os/os.h"
#include "input/input.h"
#include "sound/sound.h"

#include <memory.h>

Game::Game()
    : currentView_(VIEW_NONE)
{
    loadResources();

    memset(&viewStates_, 0, sizeof(viewStates_));

    viewStates_[VIEW_SPLASH].enter = &Game::splashEnter;
    viewStates_[VIEW_SPLASH].leave = &Game::splashLeave;
    viewStates_[VIEW_SPLASH].update = &Game::splashUpdate;
    viewStates_[VIEW_SPLASH].render = &Game::splashRender;

    viewStates_[VIEW_MAINMENU].enter = &Game::mainMenuEnter;
    viewStates_[VIEW_MAINMENU].leave = &Game::mainMenuLeave;
    viewStates_[VIEW_MAINMENU].update = &Game::mainMenuUpdate;
    viewStates_[VIEW_MAINMENU].render = &Game::mainMenuRender;

    switchView(VIEW_SPLASH);
}

Game::~Game()
{
}

void Game::update()
{
    switchedView_ = false;
    if (viewStates_[currentView_].update) {
        ((*this).*viewStates_[currentView_].update)();
    }
}

void Game::render()
{
    if (switchedView_)
        return;

    if (viewStates_[currentView_].render) {
        ((*this).*viewStates_[currentView_].render)();
    }
}

void Game::switchView(Game::View view)
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

int Game::viewElapsedMS()
{
    uint64_t diff = os::mono() - currentViewEnterTime_;
    return (int)(diff / 1000);
}

void Game::loadResources()
{
    // VIEW_SPLASH
    splashBGM_ = sound::loadOGG("data/splash.ogg", 1, false);

    // VIEW_MAINMENU
    mainMenuBGM_ = sound::loadOGG("data/mainmenu.ogg", 1, true);
    mainMenuFont_ = gfx::loadFont("yoster");
    mainMenuLogo_ = gfx::loadPNG("data/logo.png");
}

// --------------------------------------------------------------------------------------
// Splash

#define SPLASH_FADE_IN_TIME 1000
#define SPLASH_SHOW_TIME 1000
#define SPLASH_FADE_OUT_TIME 1000
#define SPLASH_TOTAL_TIME (SPLASH_FADE_IN_TIME + SPLASH_SHOW_TIME + SPLASH_FADE_OUT_TIME)

void Game::splashEnter()
{
    sound::play(splashBGM_);
}

void Game::splashLeave()
{
    sound::stop(splashBGM_);
}

void Game::splashUpdate()
{
    if ((viewElapsedMS() > SPLASH_TOTAL_TIME)
        || input::pressed(input::START)
        || input::pressed(input::ACCEPT)
        || input::pressed(input::CANCEL))
    {
        switchView(VIEW_MAINMENU);
    }
}

void Game::splashRender()
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

void Game::mainMenuEnter()
{
    mainMenuIndex_ = 0;

    sound::play(mainMenuBGM_);
}

void Game::mainMenuLeave()
{
    sound::stop(mainMenuBGM_);
}

void Game::mainMenuUpdate()
{
    if(input::pressed(input::UP)) {
        --mainMenuIndex_;
        if(mainMenuIndex_ < 0)
            mainMenuIndex_ = 2;
    }
    if(input::pressed(input::DOWN)) {
        mainMenuIndex_ = (mainMenuIndex_ + 1) % 3;
    }

    if(input::released(input::ACCEPT)) {
        switch(mainMenuIndex_) {
            case 2: // Quit
                os::quit();
                break;
        }
    }
}

void Game::mainMenuRender()
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
    y += menuFontSize * 1.5f;
    gfx::drawText((float)os::windowWidth() / 2, y, "New Game", mainMenuFont_, menuFontSize, (mainMenuIndex_ == 1) ? &selectedColor : &unselectedColor);
    y += menuFontSize * 1.5f;
    gfx::drawText((float)os::windowWidth() / 2, y, "Quit", mainMenuFont_, menuFontSize, (mainMenuIndex_ == 2) ? &selectedColor : &unselectedColor);
}
