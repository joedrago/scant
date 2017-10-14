#include "gfx/gfx.h"
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
}

void update()
{
    if(input::released(input::START)) {
        os::quit();
    }

    if(input::pressed(input::ACCEPT)) {
        sound::play(soundID_);
    }

    if(input::pressed(input::CANCEL)) {
        // sound::stopAll();
        sound::stop(soundID_);
    }

    if(input::held(input::LEFT)) {
        loadedX_ -= 1.0f;
    }
    if(input::held(input::RIGHT)) {
        loadedX_ += 1.0f;
    }
    if(input::held(input::UP)) {
        loadedY_ -= 1.0f;
    }
    if(input::held(input::DOWN)) {
        loadedY_ += 1.0f;
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

    gfx::draw(loadedX_, loadedY_, (float)loadedSource_.w, (float)loadedSource_.h, &loadedSource_);
}

void shutdown()
{
}

}
