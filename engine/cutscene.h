#ifndef CUTSCENE_H
#define CUTSCENE_H

#include "gfx/gfx.h"
#include "gfx/spritesheet.h"

#include <map>
#include <string>
#include <vector>

class Cutscene
{
public:
    Cutscene();
    ~Cutscene();

    bool loadScenes(const char * artBasename, const char * scenesFilename, int font);

    void startScene(const char * sceneName);
    bool update(); // returns false when the cutscene is done
    void render();

    enum State
    {
        STATE_NONE = 0,
        STATE_FADEIN,
        STATE_TYPING,
        STATE_WAIT,
        STATE_FADEOUT
    };

    int frameElapsedMS();
    int stateElapsedMS();
    bool nextFrame();
    void switchState(State state);

    struct Render
    {
        Render();

        std::string src; // if empty, its a fill
        gfx::Color color;

        // In 0-1 normalized coords
        float x;
        float y;

        // 0-1 (to be multiplied by os::winWf() / os::winHf())
        float w;
        float h;

        // anchor
        float ax;
        float ay;
    };

    struct Frame
    {
        Frame();

        int fadeIn;
        int duration; // Between fadeIn and fadeOut, starts when dialogue finishes typing. if 0, waits for keypress before fading out
        int fadeOut;

        std::vector<Render *> renders;

        std::vector<std::string> dialogue;
        gfx::Color dialogueTextColor;
        float dialogueTextSize;
        float dialogueSlice; // 0-1 in height of the bottom of the screen to take up
        int dialogueSpeed; // in millseconds per char

        std::string centerText;
        gfx::Color centerTextColor;
        float centerTextSize;

        std::string bgm;
    };
    typedef std::vector<Frame *> FrameList;

    struct Scene
    {
        FrameList frames;
    };

protected:
    bool end();

    int font_;
    gfx::Spritesheet spritesheet_;
    uint64_t frameStartTime_;
    uint64_t stateStartTime_;
    Scene * currentScene_;
    State state_;
    int frameIndex_;
    int bgm_;
    std::map<std::string, Scene *> scenes_;
};

#endif // ifndef CUTSCENE_H
