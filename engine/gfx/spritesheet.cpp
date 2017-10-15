#include "gfx/spritesheet.h"

#include "os/os.h"

#include "cJSON.h"

namespace gfx
{

Spritesheet::Spritesheet()
{
}

Spritesheet::~Spritesheet()
{
    sprites_.clear();
}

int jsonGetInt(cJSON * obj, const char * name)
{
    if (!cJSON_IsObject(obj))
        return 0;
    cJSON * i = cJSON_GetObjectItem(obj, name);
    if (!i)
        return 0;
    if (!cJSON_IsNumber(i))
        return 0;
    return i->valueint;
}

const char * jsonGetString(cJSON * obj, const char * name)
{
    if (!cJSON_IsObject(obj))
        return nullptr;
    cJSON * s = cJSON_GetObjectItem(obj, name);
    if (!s)
        return nullptr;
    if (!cJSON_IsString(s))
        return nullptr;
    return s->valuestring;
}

bool Spritesheet::load(const std::string & basename)
{
    char path[512];
    sprintf(path, "data/%s.png", basename.c_str());
    gfx::TextureMetrics metrics;
    int textureId = gfx::loadPNG(path, &metrics);
    if (textureId == -1) {
        return false;
    }

    sprintf(path, "data/%s.json", basename.c_str());
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
            const char * name = jsonGetString(child, "name");
            if (name != nullptr) {
                gfx::DrawSource src;
                src.textureId = textureId;
                src.x = jsonGetInt(child, "x");
                src.y = jsonGetInt(child, "y");
                src.w = jsonGetInt(child, "w");
                src.h = jsonGetInt(child, "h");
                sprites_[name] = src;
            }
        }
    }

    return true;
}

bool Spritesheet::findSprite(const std::string & name, gfx::DrawSource & outSource)
{
    std::map<std::string, gfx::DrawSource>::iterator it = sprites_.find(name);
    if (it == sprites_.end())
        return false;

    outSource = it->second;
    return true;
}

bool Spritesheet::findCycle(const std::string & name, gfx::Cycle & outCycle)
{
    char spriteName[512];
    int index = 0;
    gfx::DrawSource src;
    for (;;) {
        sprintf(spriteName, "%s%d", name.c_str(), index);
        if (findSprite(spriteName, src)) {
            outCycle.push_back(src);
            ++index;
        } else {
            break;
        }
    }
    return outCycle.size() > 0;
}

}
