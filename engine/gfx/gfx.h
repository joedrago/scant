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

struct TextureMetrics
{
    int width;
    int height;
    int pitch; // bytes
};

int createTexture(int width, int height); // returns id
unsigned char * lockTexture(int id, TextureMetrics * outMetrics);
void unlockTexture(int id);

void draw(float pixelX, float pixelY, float pixelW, float pixelH,
          DrawSource * source = nullptr,
          Color * color = nullptr,
          float anchorX = 0.0f, float anchorY = 0.0f, float r = 0.0f);

}

#endif // ifndef GFX_GFX_H
