#include "engine.h"

#include "os/os.h"
#include "gfx/gfx.h"
#include "input/input.h"
#include "sound/sound.h"

namespace engine
{

bool startup()
{
    os::startup();
    if(!gfx::startup()) {
        os::shutdown();
        return false;
    }
    input::startup();
    sound::startup();
    return true;
}

void shutdown()
{
    sound::shutdown();
    input::shutdown();
    gfx::shutdown();
    os::shutdown();
}

void begin()
{
    input::update();
    sound::update();
    gfx::begin();
}

void end()
{
    gfx::end();
}

}
