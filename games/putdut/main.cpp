#include "gfx/gfx.h"
#include "os/os.h"

#include "Game.h"

namespace game
{

static Game * game_;

void configure()
{
    os::setWindowName("PutDut");
    os::setWindowFullscreen(false);
}

void startup()
{
    game_ = new Game();
}

void update()
{
    game_->update();
    game_->render();
}

void shutdown()
{
    delete game_;
    game_ = nullptr;
}

}
