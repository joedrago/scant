#include "Game.h"

#include "gfx/gfx.h"
#include "os/os.h"
#include "input/input.h"
#include "sound/sound.h"

#include <memory.h>

Game::Game()
    : currentState_(STATE_NONE)
{
    loadResources();

    memset(&stateFuncs_, 0, sizeof(stateFuncs_));

    stateFuncs_[STATE_MOVE].enter = &Game::moveEnter;
    stateFuncs_[STATE_MOVE].leave = &Game::moveLeave;
    stateFuncs_[STATE_MOVE].update = &Game::moveUpdate;
    stateFuncs_[STATE_MOVE].render = &Game::moveRender;

    switchState(STATE_MOVE);
}

Game::~Game()
{
}

void Game::enter()
{
}

void Game::leave()
{
}

void Game::update()
{
    switchedState_ = false;
    if (stateFuncs_[currentState_].update) {
        ((*this).*stateFuncs_[currentState_].update)();
    }
}

void Game::render()
{
    if (switchedState_)
        return;

    if (stateFuncs_[currentState_].render) {
        ((*this).*stateFuncs_[currentState_].render)();
    }
}

void Game::switchState(Game::State state)
{
    if (stateFuncs_[currentState_].leave) {
        ((*this).*stateFuncs_[currentState_].leave)();
    }
    currentState_ = state;
    if (stateFuncs_[currentState_].enter) {
        ((*this).*stateFuncs_[currentState_].enter)();
    }
    switchedState_ = true;
    currentStateEnterTime_ = os::mono();
}

int Game::stateElapsedMS()
{
    uint64_t diff = os::mono() - currentStateEnterTime_;
    return (int)(diff / 1000);
}

void Game::loadResources()
{
    debugFont_ = gfx::loadFont("yoster");
}

// --------------------------------------------------------------------------------------
// Splash

void Game::moveEnter()
{
}

void Game::moveLeave()
{
}

void Game::moveUpdate()
{
}

void Game::moveRender()
{
    float fontSize = 0.05f * os::windowHeight();
    gfx::Color textColor = { 255, 0, 255, 255 };
    gfx::drawText((float)os::windowWidth() / 2, (float)os::windowHeight() / 2, "Insert Game Here", debugFont_, fontSize, &textColor);
}
