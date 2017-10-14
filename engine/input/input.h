#ifndef INPUT_INPUT_H
#define INPUT_INPUT_H

namespace input
{

enum Button
{
    UP    = (1 << 0),
    DOWN  = (1 << 1),
    LEFT  = (1 << 2),
    RIGHT = (1 << 3),

    START = (1 << 4),
    ACCEPT = (1 << 5),
    CANCEL = (1 << 6)
};

void startup();
void shutdown();
void update();

bool pressed(int button);
bool released(int button);
bool held(int button);

}

#endif
