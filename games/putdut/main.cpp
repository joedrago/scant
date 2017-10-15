#include "gfx/gfx.h"
#include "os/os.h"

#include "App.h"

namespace game
{

static App * app_;

void configure()
{
    os::setWindowName("PutDut");
    os::setWindowFullscreen(false);
}

void startup()
{
    app_ = new App();
}

void update()
{
    app_->update();
    app_->render();
}

void shutdown()
{
    delete app_;
    app_ = nullptr;
}

}
