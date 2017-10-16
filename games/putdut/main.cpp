#include "gfx/gfx.h"
#include "os/os.h"

#include "App.h"

namespace game
{

static App * app_;

void configure()
{
    os::setWindowName("PutDut");
#if defined(_DEBUG)
    os::setWindowFullscreen(false);
#else
    os::setWindowFullscreen(true);
#endif
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
