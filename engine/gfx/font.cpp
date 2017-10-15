#include "gfx/font.h"

#include "gfx/gfx.h"
#include "os/os.h"

#include "cJSON.h"

#include <stdio.h>

namespace gfx
{

// In spritesheet.cpp, I'm very lazy right now.
extern int jsonGetInt(cJSON * obj, const char * name);
extern const char * jsonGetString(cJSON * obj, const char * name);

Font::Font()
    : textureId_(-1)
    , maxHeight_(0)
{
}

Font::~Font()
{
}

bool Font::load(const char * basename)
{
    char path[512];
    sprintf(path, "data/%s.png", basename);
    gfx::TextureMetrics metrics;
    int textureId = gfx::loadPNG(path, &metrics);
    if (textureId == -1) {
        return false;
    }
    textureId_ = textureId;

    sprintf(path, "data/%s.json", basename);
    std::string rawJSON;
    if (!os::readFile(path, rawJSON)) {
        return false;
    }

    cJSON * json = cJSON_Parse(rawJSON.c_str());
    if (!json) {
        return false;
    }

    if (cJSON_IsArray(json)) {
        cJSON * child = json->child;
        for (; child != nullptr; child = child->next) {
            Glyph glyph;
            glyph.src.textureId = textureId_;
            glyph.src.x    = jsonGetInt(child, "x");
            glyph.src.y    = jsonGetInt(child, "y");
            glyph.src.w    = jsonGetInt(child, "width");
            glyph.src.h    = jsonGetInt(child, "height");
            glyph.id       = jsonGetInt(child, "id");
            glyph.xoffset  = jsonGetInt(child, "xoffset");
            glyph.yoffset  = jsonGetInt(child, "yoffset");
            glyph.xadvance = jsonGetInt(child, "xadvance");
            if (glyph.id > 0) {
                glyphs_[glyph.id] = glyph;
                if(maxHeight_ < glyph.src.h) {
                    maxHeight_ = glyph.src.h;
                }
            }
        }
    }
    return true;
}

gfx::Font::Glyph * Font::findGlyph(int id)
{
    std::map<int, Glyph>::iterator it = glyphs_.find(id);
    if (it == glyphs_.end())
        return nullptr;

    return &it->second;
}

}
