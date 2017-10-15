#include "gfx/gfx.h"
#include "gfx/spritesheet.h"
#include "input/input.h"
#include "os/os.h"
#include "sound/sound.h"

namespace game
{

int homemadeID_;

gfx::DrawSource loadedSource_;
int loadedID_;
float loadedX_ = 400.0f;
float loadedY_ = 300.0f;

int soundID_;
int fontID_;

enum Direction
{
    DIRECTION_UP = 0,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,

    DIRECTION_COUNT
};
gfx::DrawSource idleSources_[DIRECTION_COUNT];
gfx::Cycle walkCycles_[DIRECTION_COUNT];

Direction facing_ = DIRECTION_DOWN;
bool walking_ = false;

void configure()
{
    os::setWindowName("Scant Tests");
    os::setWindowFullscreen(false);
}

void startup()
{
    homemadeID_ = gfx::createTexture(256, 256);

    gfx::TextureMetrics metrics;
    unsigned char * pixels = gfx::lockTexture(homemadeID_, &metrics);
    for (int j = 0; j < 256; ++j) {
        for (int i = 0; i < 256; ++i) {
            unsigned char * pixel = pixels + (i * 4) + (j * metrics.pitch);
            pixel[0] = 255;
            pixel[1] = i;
            pixel[2] = j;
            pixel[3] = 255;
        }
    }
    gfx::unlockTexture(homemadeID_);

    loadedSource_.textureId = gfx::loadPNG("data/happy.png", &metrics);
    loadedSource_.x = 0;
    loadedSource_.y = 0;
    loadedSource_.w = metrics.width;
    loadedSource_.h = metrics.height;

    int bgm = sound::loadOGG("data/loop1.ogg", 1, true);
    sound::play(bgm);

    soundID_ = sound::loadOGG("data/horn.ogg", 2, false);

    gfx::Spritesheet sheet;
    sheet.load("art");
    sheet.findSprite("player_up_idle", idleSources_[DIRECTION_UP]);
    sheet.findSprite("player_down_idle", idleSources_[DIRECTION_DOWN]);
    sheet.findSprite("player_left_idle", idleSources_[DIRECTION_LEFT]);
    sheet.findSprite("player_right_idle", idleSources_[DIRECTION_RIGHT]);
    sheet.findCycle("player_up_walk", walkCycles_[DIRECTION_UP]);
    sheet.findCycle("player_down_walk", walkCycles_[DIRECTION_DOWN]);
    sheet.findCycle("player_left_walk", walkCycles_[DIRECTION_LEFT]);
    sheet.findCycle("player_right_walk", walkCycles_[DIRECTION_RIGHT]);

    fontID_ = gfx::loadFont("yoster");
}

void update()
{
    if (input::released(input::START)) {
        os::quit();
    }

    if (input::pressed(input::ACCEPT)) {
        sound::play(soundID_);
    }

    if (input::pressed(input::CANCEL)) {
        // sound::stopAll();
        sound::stop(soundID_);
    }

    walking_ = false;

    if (input::held(input::LEFT)) {
        loadedX_ -= 1.0f;
        facing_ = DIRECTION_LEFT;
        walking_ = true;
    }
    if (input::held(input::RIGHT)) {
        loadedX_ += 1.0f;
        facing_ = DIRECTION_RIGHT;
        walking_ = true;
    }
    if (input::held(input::UP)) {
        loadedY_ -= 1.0f;
        facing_ = DIRECTION_UP;
        walking_ = true;
    }
    if (input::held(input::DOWN)) {
        loadedY_ += 1.0f;
        facing_ = DIRECTION_DOWN;
        walking_ = true;
    }

    gfx::Color c1 = { 255, 128, 0, 255 };
    gfx::draw(10, 10, 100, 100, nullptr, &c1);

    gfx::Color c2 = { 255, 0, 128, 255 };
    gfx::draw(10, 300, 50, 200, nullptr, &c2);

    gfx::DrawSource src;
    src.textureId = homemadeID_;
    src.x = 0;
    src.y = 0;
    src.w = 256;
    src.h = 256;
    gfx::draw(400, 10, 256, 256, &src);

    gfx::draw(400, 300, (float)loadedSource_.w, (float)loadedSource_.h, &loadedSource_);

    int stepIndex = 0;
    if ((GetTickCount() % 500) > 250) {
        stepIndex = 1;
    }

    gfx::DrawSource * playerSrc = &idleSources_[facing_];
    if (walking_) {
        playerSrc = &walkCycles_[facing_][stepIndex];
    }
    gfx::draw(loadedX_, loadedY_, (float)playerSrc->w, (float)playerSrc->h, playerSrc);

    gfx::Color textColor = { 255, 255, 255, 255 };
    gfx::drawText((float)os::windowWidth() / 2, (float)os::windowHeight() / 2, "Hello Yaya", fontID_, 100.0f, &textColor);

    gfx::Color red = { 255, 0, 0, 255 };
    draw((float)os::windowWidth() / 2, (float)os::windowHeight() / 2, 1, 1, nullptr, &red);
    draw((float)os::windowWidth()-1, (float)os::windowHeight()-1, 1, 1, nullptr, &red);
}

void shutdown()
{
}

}
