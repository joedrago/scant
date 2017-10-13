#include "gfx/gfx.h"
#include "os/os.h"

namespace game
{

void configure()
{
    os::setWindowName("Scant Tests");
    os::setWindowFullscreen(false);
}

void startup()
{
}

void update()
{
    gfx::Color c = {255, 128, 0, 255};
    gfx::draw(10,10,100,100, nullptr, &c);
}

void shutdown()
{
}

}
