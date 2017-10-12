#include "engine.h"

#include "os/os.h"
#include "gfx/gfx.h"
#include "input/input.h"
#include "sound/sound.h"

namespace engine
{

void startup()
{
    os::startup();
    gfx::startup();
    input::startup();
    sound::startup();
}

void shutdown()
{
    sound::shutdown();
    input::shutdown();
    gfx::shutdown();
    os::shutdown();
}

}
