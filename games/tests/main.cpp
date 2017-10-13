#include "gfx/gfx.h"
#include "os/os.h"

namespace game
{

int textureID_;

void configure()
{
    os::setWindowName("Scant Tests");
    os::setWindowFullscreen(false);
}

void startup()
{
    textureID_ = gfx::createTexture(256, 256);

    gfx::TextureMetrics metrics;
    unsigned char * pixels = gfx::lockTexture(textureID_, &metrics);
    for (int j = 0; j < 256; ++j) {
        for (int i = 0; i < 256; ++i) {
            unsigned char * pixel = pixels + (i * 4) + (j * metrics.pitch);
            pixel[0] = 255;
            pixel[1] = 0;
            pixel[2] = j;
            pixel[3] = 255;
        }
    }
    gfx::unlockTexture(textureID_);
}

void update()
{
    gfx::Color c1 = { 255, 128, 0, 255 };
    gfx::draw(10, 10, 100, 100, nullptr, &c1);

    gfx::Color c2 = { 255, 0, 128, 255 };
    gfx::draw(300, 300, 50, 200, nullptr, &c2);

    gfx::DrawSource src;
    src.textureId = textureID_;
    src.x = 0;
    src.y = 0;
    src.w = 256;
    src.h = 256;
    gfx::draw(400, 10, 256, 256, &src);
}

void shutdown()
{
}

}
