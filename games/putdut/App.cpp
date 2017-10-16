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

    viewStates_[VIEW_FINALE].enter = &App::finaleEnter;
    viewStates_[VIEW_FINALE].leave = &App::finaleLeave;
    viewStates_[VIEW_FINALE].update = &App::finaleUpdate;
    viewStates_[VIEW_FINALE].render = &App::finaleRender;

    game_ = new Game(this);
    gotoIndex_ = game_->highestLevelReached();

#if _DEBUG
    switchView(VIEW_MAINMENU);
#else
    switchView(VIEW_SPLASH);
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

    // VIEW_FINALE
    finaleBGM_ = sound::loadOGG("data/finale.ogg", 1, true);
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
    gfx::drawText(os::winWf() / 2, os::winHf() / 2, "Drago Family Games presents...", mainMenuFont_, fontSize, &textColor);
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

// --------------------------------------------------------------------------------------
// Finale

void App::finaleEnter()
{
    sound::play(finaleBGM_);
}

void App::finaleLeave()
{
    sound::stop(finaleBGM_);
}

void App::finaleUpdate()
{
    if (viewElapsedMS() < 250)
        return;

    if (input::pressed(input::ACCEPT) || input::pressed(input::CANCEL) || input::pressed(input::START)) {
        switchView(VIEW_MAINMENU);
    }
}

void App::finaleRender()
{
    float fontSize = 0.05f * os::windowHeight();
    gfx::Color textColor = { 255, 0, 255, 255 };
    gfx::drawText((float)os::windowWidth() / 2, (float)os::windowHeight() / 2, "Insert Finale Here", mainMenuFont_, fontSize, &textColor);
    gfx::drawText(os::winWf() / 2.0f, (fontSize * 2) + (os::winHf() / 2.0f), "Press any key for main menu", mainMenuFont_, fontSize, &textColor);
}
