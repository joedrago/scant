#ifndef GAME_H
#define GAME_H

#include "gfx/gfx.h"

#include <stdint.h>

class Game
{
public:
    enum State
    {
        STATE_NONE = 0,

        STATE_MOVE,

        STATE_COUNT
    };

    Game();
    ~Game();

    void enter();
    void leave();
    void update();
    void render();
    void switchState(State state);
    int stateElapsedMS();

    void loadResources();

    void moveEnter();
    void moveLeave();
    void moveUpdate();
    void moveRender();

    typedef void (Game::*enterFunction)();
    typedef void (Game::*leaveFunction)();
    typedef void (Game::*updateFunction)();
    typedef void (Game::*renderFunction)();

    struct StateFuncs
    {
        enterFunction enter;
        leaveFunction leave;
        updateFunction update;
        renderFunction render;
    };

protected:
    State currentState_;
    uint64_t currentStateEnterTime_;
    StateFuncs stateFuncs_[STATE_COUNT];
    bool switchedState_;

    int debugFont_;
};

#endif
