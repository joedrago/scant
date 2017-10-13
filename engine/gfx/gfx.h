#ifndef GFX_GFX_H
#define GFX_GFX_H

namespace gfx
{

bool startup();
void shutdown();

void begin();
void end();

struct Color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct DrawSource
{
    int textureId;
    int x;
    int y;
    int w;
    int h;
};

void draw(float pixelX, float pixelY, float pixelW, float pixelH,
    DrawSource *source = nullptr,
    Color *color = nullptr,
    float anchorX = 0.0f, float anchorY = 0.0f, float r = 0.0f);

}

#endif
