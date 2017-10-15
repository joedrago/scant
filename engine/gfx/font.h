#ifndef GFX_FONT_H
#define GFX_FONT_H

#include "gfx/gfx.h"

#include <map>

namespace gfx
{

class Font
{
public:
    Font();
    ~Font();

    bool load(const char * name);

    struct Glyph
    {
        int id;
        int xoffset;
        int yoffset;
        int xadvance;
        gfx::DrawSource src;
    };

    Glyph * Font::findGlyph(int id);

    inline int maxHeight() { return maxHeight_; }

private:
    int textureId_;
    int maxHeight_;
    std::map<int, Glyph> glyphs_;
};

}

#endif // ifndef GFX_FONT_H
