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
    gfx::Color c1 = {255, 128, 0, 255};
    gfx::draw(10,10,100,100, nullptr, &c1);

    gfx::Color c2 = {255, 0, 128, 255};
    gfx::draw(300,300,50,200, nullptr, &c2);
}

void shutdown()
{
}

}
