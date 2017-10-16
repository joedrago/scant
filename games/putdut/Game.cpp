#include "Game.h"

#include "App.h"

#include "gfx/gfx.h"
#include "gfx/spritesheet.h"
#include "os/os.h"
#include "input/input.h"
#include "sound/sound.h"

#include "cJSON.h"

#include <assert.h>
#include <memory.h>

#ifdef _DEBUG
#define DRAW_STATE_NAME
#endif

#define LOGO_HEIGHT (0.1f)

Game::Game(App * app)
    : currentState_(STATE_NONE)
    , app_(app)
{
    loadResources();

    memset(&stateFuncs_, 0, sizeof(stateFuncs_));

    stateFuncs_[STATE_NEWLEVEL].enter = &Game::newLevelEnter;
    stateFuncs_[STATE_NEWLEVEL].leave = &Game::newLevelLeave;
    stateFuncs_[STATE_NEWLEVEL].update = &Game::newLevelUpdate;
    stateFuncs_[STATE_NEWLEVEL].render = &Game::newLevelRender;

    stateFuncs_[STATE_PAUSEMENU].enter = &Game::pauseMenuEnter;
    stateFuncs_[STATE_PAUSEMENU].leave = &Game::pauseMenuLeave;
    stateFuncs_[STATE_PAUSEMENU].update = &Game::pauseMenuUpdate;
    stateFuncs_[STATE_PAUSEMENU].render = &Game::pauseMenuRender;

    stateFuncs_[STATE_FANFARE].enter = &Game::fanfareEnter;
    stateFuncs_[STATE_FANFARE].leave = &Game::fanfareLeave;
    stateFuncs_[STATE_FANFARE].update = &Game::fanfareUpdate;
    stateFuncs_[STATE_FANFARE].render = &Game::fanfareRender;

    stateFuncs_[STATE_CONFIRMNEXT].enter = &Game::confirmNextEnter;
    stateFuncs_[STATE_CONFIRMNEXT].leave = &Game::confirmNextLeave;
    stateFuncs_[STATE_CONFIRMNEXT].update = &Game::confirmNextUpdate;
    stateFuncs_[STATE_CONFIRMNEXT].render = &Game::confirmNextRender;

    stateFuncs_[STATE_IDLE].enter = &Game::idleEnter;
    stateFuncs_[STATE_IDLE].leave = &Game::idleLeave;
    stateFuncs_[STATE_IDLE].update = &Game::idleUpdate;
    stateFuncs_[STATE_MOVE].enter = &Game::moveEnter;
    stateFuncs_[STATE_MOVE].leave = &Game::moveLeave;
    stateFuncs_[STATE_MOVE].update = &Game::moveUpdate;

    switchedLevelOnce_ = false;
}

Game::~Game()
{
    for (std::vector<Level *>::iterator it = levels_.begin(); it != levels_.end(); ++it) {
        delete *it;
    }
    levels_.clear();
}

void Game::enter()
{
    if (!switchedLevelOnce_) {
        switchLevel(0);
    }
}

void Game::leave()
{
}

void Game::update()
{
    do {
        switchedState_ = false;
        if (stateFuncs_[currentState_].update) {
            ((*this).*stateFuncs_[currentState_].update)();
        }
    } while (switchedState_);
}

void Game::render()
{
    renderLevel();

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
    font_ = gfx::loadFont("yoster");

    soundNewLevel_ = sound::loadOGG("data/newlevel.ogg", 1, false);
    soundStep_ = sound::loadOGG("data/step.ogg", 2, false);
    soundReset_ = sound::loadOGG("data/reset.ogg", 1, false);
    soundFanfare_ = sound::loadOGG("data/fanfare.ogg", 1, false);
    bgmConfirm_ = sound::loadOGG("data/confirm.ogg", 1, true);
    bgmGame_ = sound::loadOGG("data/game.ogg", 1, true);
    bgmGamePlaying_ = false;

    loadLevels();

    gfx::Spritesheet sheet;
    sheet.load("art");
    sheet.findCycle("wall", artWalls_);
    sheet.findCycle("floor", artFloors_);
    sheet.findSprite("dest", artDest_);
    sheet.findSprite("happy", happyFace_);
    sheet.findCycle("box", artBoxes_);

    sheet.findSprite("player_up_idle", idleSources_[DIRECTION_UP]);
    sheet.findSprite("player_down_idle", idleSources_[DIRECTION_DOWN]);
    sheet.findSprite("player_left_idle", idleSources_[DIRECTION_LEFT]);
    sheet.findSprite("player_right_idle", idleSources_[DIRECTION_RIGHT]);
    sheet.findCycle("player_up_walk", walkCycles_[DIRECTION_UP]);
    sheet.findCycle("player_down_walk", walkCycles_[DIRECTION_DOWN]);
    sheet.findCycle("player_left_walk", walkCycles_[DIRECTION_LEFT]);
    sheet.findCycle("player_right_walk", walkCycles_[DIRECTION_RIGHT]);

    cellSize_ = min(os::winWf() / Level::MAX_W, os::winHf() / Level::MAX_H);
    gameOffsetX_ = (os::winWf() - (cellSize_ * Level::MAX_W)) / 2.0f;
    gameOffsetY_ = (os::winHf() - (cellSize_ * Level::MAX_H)) / 2.0f;

    // leave room for title at top
    cellSize_ *= (1.0f - LOGO_HEIGHT);
    gameOffsetY_ += os::winHf() * LOGO_HEIGHT;

    walkSpeed_ = WALK_SLOW;
}

void Game::loadLevels()
{
    std::string jsonString;
    os::readFile("data/levels.json", jsonString);
    cJSON * json = cJSON_Parse(jsonString.c_str());
    if (cJSON_IsArray(json)) {
        cJSON * child = json->child;
        for (; child != nullptr; child = child->next) {
            // const char * title = jsonGetString(child, "title");
            std::string title;
            cJSON * jsonTitle = cJSON_GetObjectItem(child, "title");
            if (cJSON_IsString(jsonTitle)) {
                title = jsonTitle->valuestring;
            }
            cJSON * jsonLines = cJSON_GetObjectItem(child, "lines");
            if (jsonLines && cJSON_IsArray(jsonLines)) {
                std::vector<std::string> lines;
                for (cJSON * jsonLine = jsonLines->child; jsonLine; jsonLine = jsonLine->next) {
                    assert(cJSON_IsString(jsonLine));
                    lines.push_back(jsonLine->valuestring);
                }

                Level * level = new Level();
                if (level->parse(lines)) {
                    level->title_ = title;
                    levels_.push_back(level);
                } else {
                    assert(0);
                    delete level;
                }
            }
        }
    }
    cJSON_Delete(json);
}

void Game::switchLevel(int index)
{
    switchedLevelOnce_ = true;

    assert(index >= 0);
    assert(index < (int)levels_.size());
    currentLevelIndex_ = index;

    resetLevel();

    switchState(STATE_NEWLEVEL);
}

void Game::resetLevel()
{
    currentLevel_.copy(*levels_[currentLevelIndex_]);

    playerX_ = currentLevel_.startX_;
    playerY_ = currentLevel_.startY_;
    playerFacing_ = DIRECTION_DOWN;
    playerDrawX_ = gameOffsetX_ + (playerX_ * cellSize_);
    playerDrawY_ = gameOffsetY_ + (playerY_ * cellSize_);
}

void Game::enableGameBGM(bool playing)
{
    if (bgmGamePlaying_ != playing) {
        bgmGamePlaying_ = playing;
        if (bgmGamePlaying_) {
            sound::play(bgmGame_);
        } else {
            sound::stop(bgmGame_);
        }
    }
}

bool Game::calcMovePos(Direction dir, int x, int y, int & ox, int & oy)
{
    switch (dir) {
        case DIRECTION_UP:
            ox = x;
            oy = y - 1;
            if (oy < 0)
                return false;
            break;
        case DIRECTION_DOWN:
            ox = x;
            oy = y + 1;
            if (oy >= Level::MAX_H)
                return false;
            break;
        case DIRECTION_LEFT:
            ox = x - 1;
            oy = y;
            if (ox < 0)
                return false;
            break;
        case DIRECTION_RIGHT:
            ox = x + 1;
            oy = y;
            if (ox > Level::MAX_W)
                return false;
            break;
    }
    return true;
}

void Game::move(Game::Direction dir)
{
    playerFacing_ = dir;

    int dstX, dstY;
    if (!calcMovePos(dir, playerX_, playerY_, dstX, dstY)) {
        return;
    }

    Level::Cell & curCell = currentLevel_.cells_[playerX_][playerY_];
    Level::Cell & dstCell = currentLevel_.cells_[dstX][dstY];
    if (dstCell.wall_)
        return;

    int boxDstX = -1;
    int boxDstY = -1;

    if (dstCell.box_) {
        if (!calcMovePos(dir, dstX, dstY, boxDstX, boxDstY)) {
            return;
        }
        Level::Cell & boxDstCell = currentLevel_.cells_[boxDstX][boxDstY];
        if (boxDstCell.wall_)
            return;
        if (boxDstCell.box_)
            return;
    }

    moving_ = true;
    if (boxDstX == -1) {
        moveAction_.boxX_ = -1;
        moveAction_.boxY_ = -1;
        moveAction_.boxTravelX_ = -1;
        moveAction_.boxTravelY_ = -1;
    } else {
        moveAction_.boxX_ = dstX;
        moveAction_.boxY_ = dstY;
        moveAction_.boxTravelX_ = boxDstX;
        moveAction_.boxTravelY_ = boxDstY;
    }
    moveAction_.playerX_ = playerX_;
    moveAction_.playerY_ = playerY_;
    moveAction_.playerTravelX_ = dstX;
    moveAction_.playerTravelY_ = dstY;
    moveAction_.playerFacing_ = playerFacing_;
    switchState(STATE_MOVE);

    sound::play(soundStep_);
}

// --------------------------------------------------------------------------------------
// NewLevel

void Game::newLevelEnter()
{
    sound::play(soundNewLevel_);
}

void Game::newLevelLeave()
{
    enableGameBGM(true);
}

void Game::newLevelUpdate()
{
}

void Game::newLevelRender()
{
    if (stateElapsedMS() > 1000) {
        switchState(STATE_IDLE);
    }
}

// --------------------------------------------------------------------------------------
// PauseMenu

void Game::pauseMenuEnter()
{
    enableGameBGM(false);
}

void Game::pauseMenuLeave()
{
    enableGameBGM(true);
}

void Game::pauseMenuUpdate()
{
}

void Game::pauseMenuRender()
{
}

// --------------------------------------------------------------------------------------
// Fanfare

#define FANFARE_TIME 2000

void Game::renderFanfareOverlay(float p)
{
    gfx::Color brightColor = { 255, 255, 255, (unsigned char)(128.0f * p) };
    gfx::draw(0, 0, os::winWf(), os::winHf(), nullptr, &brightColor);

    float happyH = os::winHf() * 0.4f;
    float happyW = happyFace_.w * happyH / happyFace_.h;
    gfx::Color happyColor = { 255, 255, 255, (unsigned char)(255.0f * p) };
    gfx::draw(os::winWf() / 2.0f, os::winHf() / 2.0f, happyW, happyH, &happyFace_, &happyColor, 0.5f, 0.5f);
}

void Game::fanfareEnter()
{
    enableGameBGM(false);
    sound::play(soundFanfare_);
}

void Game::fanfareLeave()
{
}

void Game::fanfareUpdate()
{
    if (stateElapsedMS() > FANFARE_TIME) {
        switchState(STATE_CONFIRMNEXT);
    }
}

void Game::fanfareRender()
{
    float p = os::clamp((float)stateElapsedMS() / FANFARE_TIME, 0.0f, 1.0f);
    renderFanfareOverlay(p);
}

// --------------------------------------------------------------------------------------
// ConfirmNext

void Game::confirmNextEnter()
{
    sound::play(bgmConfirm_);
}

void Game::confirmNextLeave()
{
    sound::stop(bgmConfirm_);
}

void Game::confirmNextUpdate()
{
    if (input::pressed(input::ACCEPT)) {
        switchLevel(currentLevelIndex_ + 1);
        switchState(STATE_NEWLEVEL);
    }
}

void Game::confirmNextRender()
{
    renderFanfareOverlay(1.0f);

    gfx::Color shadowColor = { 0, 0, 0, 255 };
    gfx::Color textColor = { 128, 192, 192, 255 };
    float fontSize = os::winHf() * 0.08f;
    float shadow = os::winHf() * 0.0125f;
    const char * text = "Press A for next level!";
    gfx::drawText(shadow + os::winWf() / 2.0f, shadow + os::winHf() * 0.8f, text, font_, fontSize, &shadowColor);
    gfx::drawText(os::winWf() / 2.0f, os::winHf() * 0.8f, text, font_, fontSize, &textColor);
}

// --------------------------------------------------------------------------------------
// Idle

void Game::idleEnter()
{
}

void Game::idleLeave()
{
}

void Game::idleUpdate()
{
    moving_ = false;
    playerDrawX_ = gameOffsetX_ + (playerX_ * cellSize_);
    playerDrawY_ = gameOffsetY_ + (playerY_ * cellSize_);

    if (input::pressed(input::START)) {
        app_->switchView(App::VIEW_MAINMENU);
        return;
    }

    if (currentLevel_.win() /* || input::pressed(input::CANCEL) */) {
        switchState(STATE_FANFARE);
        return;
    }

    if (input::pressed(input::CANCEL)) {
        sound::play(soundReset_);
        resetLevel();
        return;
    }

    if (input::held(input::ACCEPT)) {
        walkSpeed_ = WALK_FAST;
    } else {
        walkSpeed_ = WALK_SLOW;
    }

    if (input::held(input::UP)) {
        move(DIRECTION_UP);
    } else if (input::held(input::DOWN)) {
        move(DIRECTION_DOWN);
    } else if (input::held(input::LEFT)) {
        move(DIRECTION_LEFT);
    } else if (input::held(input::RIGHT)) {
        move(DIRECTION_RIGHT);
    }
}

void Game::calcLerpDraw(float p, int srcX, int srcY, int dstX, int dstY, float & drawX, float & drawY)
{
    float srcDrawX = gameOffsetX_ + (srcX * cellSize_);
    float srcDrawY = gameOffsetY_ + (srcY * cellSize_);
    float dstDrawX = gameOffsetX_ + (dstX * cellSize_);
    float dstDrawY = gameOffsetY_ + (dstY * cellSize_);
    float difDrawX = dstDrawX - srcDrawX;
    float difDrawY = dstDrawY - srcDrawY;

    drawX = srcDrawX + (difDrawX * p);
    drawY = srcDrawY + (difDrawY * p);
}

// --------------------------------------------------------------------------------------
// Move

void Game::moveEnter()
{
}

void Game::moveLeave()
{
    if ((moveAction_.boxX_ != -1) && (moveAction_.boxY_ != -1) && (moveAction_.boxTravelX_ != -1) && (moveAction_.boxTravelY_ != -1)) {
        currentLevel_.cells_[moveAction_.boxX_][moveAction_.boxY_].box_ = false;
        currentLevel_.cells_[moveAction_.boxTravelX_][moveAction_.boxTravelY_].box_ = true;
    }
    playerX_ = moveAction_.playerTravelX_;
    playerY_ = moveAction_.playerTravelY_;
    playerDrawIndex_ = 0;
    moving_ = false;
}

void Game::moveUpdate()
{
    playerDrawIndex_ = 0;
    if ((GetTickCount() % 500) > 250) {
        playerDrawIndex_ = 1;
    }

    float p = os::clamp((float)stateElapsedMS() / (float)walkSpeed_, 0.0f, 1.0f);
    calcLerpDraw(p, playerX_, playerY_, moveAction_.playerTravelX_, moveAction_.playerTravelY_, playerDrawX_, playerDrawY_);
    if (stateElapsedMS() > walkSpeed_) {
        switchState(STATE_IDLE);
    }
}

// --------------------------------------------------------------------------------------
// renderLevel

const char * Game::debugStateName() const
{
    switch (currentState_) {
        case STATE_NONE:        return "None";
        case STATE_NEWLEVEL:    return "NewLevel";
        case STATE_PAUSEMENU:   return "PauseMenu";
        case STATE_IDLE:        return "Idle";
        case STATE_MOVE:        return "Move";
        case STATE_FANFARE:     return "Fanfare";
        case STATE_CONFIRMNEXT: return "ConfirmNext";
    }
    return "Unknown";
}

void Game::renderLevel()
{
    float fontSize = 0.05f * os::winHf();
    float margin = (fontSize / 10.0f);

#if defined(DRAW_STATE_NAME)
    gfx::Color stateColor = { 64, 64, 64, 255 };
    gfx::drawText(margin, margin, debugStateName(), font_, fontSize / 2.0f, &stateColor, 0.0f, 0.0f);
#endif

    // Draw Logos
    gfx::DrawSource & logo = app_->logo();
    float logoH = os::winHf() * LOGO_HEIGHT;
    float logoW = logo.w * logoH / logo.h;
    gfx::draw(os::winWf() / 2, margin, logoW, logoH, &logo, nullptr, 0.5f, 0.0f);

    // Draw Titles (bottom corners)
    char title1[256];
    char title2[256];
    sprintf(title1, "Level %d", currentLevelIndex_ + 1);
    sprintf(title2, "\"%s\"", currentLevel_.title_.c_str());
    gfx::Color textColor = { 192, 192, 192, 255 };
    gfx::drawText(margin, os::winHf() - margin, title1, font_, fontSize, &textColor, 0.0f, 1.0f);
    gfx::drawText(os::winWf() - margin, os::winHf() - margin, title2, font_, fontSize, &textColor, 1.0f, 1.0f);

    for (int j = 0; j < Level::MAX_H; ++j) {
        for (int i = 0; i < Level::MAX_W; ++i) {
            float x = gameOffsetX_ + (i * cellSize_);
            float y = gameOffsetY_ + (j * cellSize_);
            Level::Cell & cell = currentLevel_.cells_[i][j];
            if (cell.wall_ || cell.floor_) {
                gfx::draw(x, y, cellSize_, cellSize_, &artFloors_[0]);
            }
            if (cell.box_) {
                if (!moving_
                    || (((i != moveAction_.boxX_) || (j != moveAction_.boxY_)) && ((i != moveAction_.boxTravelX_) || (j != moveAction_.boxTravelY_))))
                {
                    gfx::draw(x, y, cellSize_, cellSize_, &artBoxes_[0]);
                }
            }
            if (cell.dest_) {
                gfx::Color faded = { 255, 255, 255, 128 };
                gfx::draw(x, y, cellSize_, cellSize_, &artDest_, &faded);
            }
            if (cell.wall_) {
                gfx::draw(x, y, cellSize_, cellSize_, &artWalls_[0]);
            }
        }
    }

    if (moving_) {
        float p = os::clamp((float)stateElapsedMS() / (float)walkSpeed_, 0.0f, 1.0f);
        float x, y;
        calcLerpDraw(p, moveAction_.boxX_, moveAction_.boxY_, moveAction_.boxTravelX_, moveAction_.boxTravelY_, x, y);
        gfx::draw(x, y, cellSize_, cellSize_, &artBoxes_[0]);
    }

    gfx::DrawSource * playerSrc = &idleSources_[playerFacing_];
    bool walking = (currentState_ == STATE_MOVE);
    if (walking) {
        playerSrc = &walkCycles_[playerFacing_][playerDrawIndex_];
    }
    gfx::draw(playerDrawX_, playerDrawY_, cellSize_, cellSize_, playerSrc);
}
