#ifndef GFX_SPRITESHEET_H
#define GFX_SPRITESHEET_H

#include "gfx/gfx.h"

#include <map>

namespace gfx
{

class Spritesheet
{
public:
    Spritesheet();
    ~Spritesheet();

    bool load(const std::string & name);

    bool findSprite(const std::string & name, gfx::DrawSource & outSource);
    bool findCycle(const std::string & name, gfx::Cycle & outCycle);
private:
    std::map<std::string, gfx::DrawSource> sprites_;
};

}

#endif
