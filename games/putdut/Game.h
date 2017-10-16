#ifndef GAME_H
#define GAME_H

#include "gfx/gfx.h"

#include "Level.h"

#include <stdint.h>
#include <vector>

class App;

class Game
{
public:
    enum State
    {
        STATE_NONE = 0,

        STATE_NEWLEVEL,
        STATE_PAUSEMENU,
        STATE_IDLE,
        STATE_MOVE,
        STATE_FANFARE,
        STATE_CONFIRMNEXT,

        STATE_COUNT
    };

    // How many ms to traverse one cell
    static const int WALK_SLOW = 200;
    static const int WALK_FAST = 50;
    static const int WALK_REWIND = 50;

    Game(App * app);
    ~Game();

    void enter();
    void leave();
    void update();
    void render();
    void switchState(State state);
    int stateElapsedMS();

    void loadResources();
    void loadLevels();

    void loadProgress();
    void saveProgress();

    void switchLevel(int index);
    void resetLevel();

    void enableBGM(bool playing);
    void switchBGM(int bgm);

    void renderLevel();
    void renderFanfareOverlay(float p);

    void newLevelEnter();
    void newLevelLeave();
    void newLevelUpdate();
    void newLevelRender();

    void pauseMenuEnter();
    void pauseMenuLeave();
    void pauseMenuUpdate();
    void pauseMenuRender();

    void fanfareEnter();
    void fanfareLeave();
    void fanfareUpdate();
    void fanfareRender();

    void confirmNextEnter();
    void confirmNextLeave();
    void confirmNextUpdate();
    void confirmNextRender();

    void idleEnter();
    void idleLeave();
    void idleUpdate();

    void moveEnter();
    void moveLeave();
    void moveUpdate();

    const char * debugStateName() const;

    typedef void (Game::* enterFunction)();
    typedef void (Game::* leaveFunction)();
    typedef void (Game::* updateFunction)();
    typedef void (Game::* renderFunction)();

    struct StateFuncs
    {
        enterFunction enter;
        leaveFunction leave;
        updateFunction update;
        renderFunction render;
    };

    enum Direction
    {
        DIRECTION_UP = 0,
        DIRECTION_DOWN,
        DIRECTION_LEFT,
        DIRECTION_RIGHT,

        DIRECTION_COUNT
    };

    bool calcMovePos(Direction dir, int x, int y, int & ox, int & oy);
    void move(Direction dir);
    bool rewind();

    void calcLerpDraw(float p, int srcX, int srcY, int dstX, int dstY, float & drawX, float & drawY);

    struct MoveAction
    {
        int playerX_;
        int playerY_;
        int playerTravelX_;
        int playerTravelY_;
        int boxX_;
        int boxY_;
        int boxTravelX_;
        int boxTravelY_;
        Direction playerFacing_;
    };

    inline int highestLevelReached() { return highestLevelReached_; }
    inline Level * getLevel(int index) { return levels_[index]; }

protected:
    bool switchedLevelOnce_;
    App * app_;
    State currentState_;
    uint64_t currentStateEnterTime_;
    StateFuncs stateFuncs_[STATE_COUNT];
    bool switchedState_;

    // Art
    int font_;
    int soundNewLevel_;
    int soundReset_;
    int soundStep_;
    int soundFanfare_;
    int bgmGame_;
    int bgmConfirm_;
    int bgmRewind_;
    int bgmCurrent_;
    bool bgmPlaying_;
    gfx::Cycle artWalls_;
    gfx::Cycle artFloors_;
    gfx::DrawSource artDest_;
    gfx::Cycle artBoxes_;
    gfx::DrawSource idleSources_[DIRECTION_COUNT];
    gfx::Cycle walkCycles_[DIRECTION_COUNT];
    gfx::DrawSource happyFace_;
    float cellSize_;
    float gameOffsetX_;
    float gameOffsetY_;
    int walkSpeed_;

    // Level info
    std::vector<Level *> levels_;
    Level currentLevel_;
    int highestLevelReached_;

    // Game state
    int currentLevelIndex_;
    int playerX_;
    int playerY_;
    float playerDrawX_;
    float playerDrawY_;
    bool moving_;
    MoveAction moveAction_;
    std::vector<MoveAction> undo_;
    int playerDrawIndex_;
    Direction playerFacing_;
    // TODO: add box positions array
};

#endif // ifndef GAME_H
