#include "Cutscene.h"

#include "input/input.h"
#include "os/os.h"
#include "sound/sound.h"

#include <assert.h>

#include "cJSON.h"

Cutscene::Render::Render()
    : x(0)
    , y(0)
    , w(0)
    , h(0)
    , ax(0.5f)
    , ay(0.5f)
    , r(0.0f)

    , durationAnim(0)
    , xAnim(0.0f)
    , yAnim(0.0f)
    , wAnim(0.0f)
    , hAnim(0.0f)
    , axAnim(0.0f)
    , ayAnim(0.0f)
    , rAnim(0.0f)
{
    color = { 255, 255, 255, 255 };
}

Cutscene::Frame::Frame()
    : fadeIn(0)
    , duration(0)
    , fadeOut(0)
    , centerTextSize(0.05f)
    , dialogueSlice(0.4f)
    , dialogueTextSize(0.05f)
    , dialogueSpeed(50)
{
    clear = { 0, 0, 0, 255 };
    centerTextColor = { 255, 255, 255, 255 };
    dialogueTextColor = { 255, 255, 255, 255 };
}

Cutscene::Cutscene()
    : currentScene_(nullptr)
    , bgm_(-1)
{
}

Cutscene::~Cutscene()
{
    // TODO: actually delete this stuff
}

int Cutscene::frameElapsedMS()
{
    uint64_t diff = os::mono() - frameStartTime_;
    return (int)(diff / 1000);
}

int Cutscene::stateElapsedMS()
{
    uint64_t diff = os::mono() - stateStartTime_;
    return (int)(diff / 1000);
}

void Cutscene::switchState(State state)
{
    state_ = state;
    stateStartTime_ = os::mono();
}

bool Cutscene::nextFrame()
{
    ++frameIndex_;
    if (frameIndex_ >= (int)currentScene_->frames.size()) {
        currentScene_ = nullptr;
        return end();
    }
    Frame * frame = currentScene_->frames[frameIndex_];
    if (frame->bgm.size() > 0) {
        int bgm = -1;
        if (frame->bgm != "-") {
            bgm = sound::loadOGG(frame->bgm.c_str(), 1, true);
        }
        if (bgm_ != bgm) {
            if (bgm_ != -1) {
                sound::stop(bgm_);
            }
            bgm_ = bgm;
            if (bgm_ != -1) {
                sound::play(bgm_);
            }
        }
    }
    frameStartTime_ = os::mono();
    if (frame->fadeIn) {
        switchState(STATE_FADEIN);
    } else if (frame->dialogue.size() > 0) {
        switchState(STATE_TYPING);
    } else {
        switchState(STATE_WAIT);
    }
    return true;
}

static void jsonFillUnsignedChar(cJSON * obj, const char * name, unsigned char & out)
{
    if (!cJSON_IsObject(obj))
        return;
    cJSON * i = cJSON_GetObjectItem(obj, name);
    if (!i)
        return;
    if (!cJSON_IsNumber(i))
        return;
    out = (unsigned char)i->valueint;
}

static void jsonFillInt(cJSON * obj, const char * name, int & out)
{
    if (!cJSON_IsObject(obj))
        return;
    cJSON * i = cJSON_GetObjectItem(obj, name);
    if (!i)
        return;
    if (!cJSON_IsNumber(i))
        return;
    out = i->valueint;
}

static void jsonFillFloat(cJSON * obj, const char * name, float & out)
{
    if (!cJSON_IsObject(obj))
        return;
    cJSON * i = cJSON_GetObjectItem(obj, name);
    if (!i)
        return;
    if (!cJSON_IsNumber(i))
        return;
    out = (float)i->valuedouble;
}

static void jsonFillString(cJSON * obj, const char * name, std::string & out)
{
    if (!cJSON_IsObject(obj))
        return;
    cJSON * s = cJSON_GetObjectItem(obj, name);
    if (!s)
        return;
    if (!cJSON_IsString(s))
        return;
    out = s->valuestring;
}

static void jsonFillColor(cJSON * obj, const char * name, gfx::Color & out)
{
    if (!cJSON_IsObject(obj))
        return;
    cJSON * jsonColor = cJSON_GetObjectItem(obj, name);
    if (!jsonColor)
        return;
    if (!cJSON_IsObject(jsonColor))
        return;

    jsonFillUnsignedChar(jsonColor, "r", out.r);
    jsonFillUnsignedChar(jsonColor, "g", out.g);
    jsonFillUnsignedChar(jsonColor, "b", out.b);
    jsonFillUnsignedChar(jsonColor, "a", out.a);
}

bool Cutscene::loadScenes(const char * artBasename, const char * scenesFilename, int font)
{
    spritesheet_.load(artBasename);
    font_ = font;

    std::string rawJSON;
    if (!os::readFile(scenesFilename, rawJSON)) {
        assert(0);
    }
    cJSON_Minify(&rawJSON[0]); // removes comments

    cJSON * json = cJSON_Parse(rawJSON.c_str());
    assert(json);
    assert(cJSON_IsObject(json));

    for (cJSON * jsonScene = json->child; jsonScene; jsonScene = jsonScene->next) {
        char * sceneName = jsonScene->string;
        Scene * scene = new Scene;
        scenes_[sceneName] = scene;

        cJSON * jsonFrames = cJSON_GetObjectItem(jsonScene, "frames");
        assert(cJSON_IsArray(jsonFrames));
        for (cJSON * jsonFrame = jsonFrames->child; jsonFrame; jsonFrame = jsonFrame->next) {
            Frame * frame = new Frame;
            scene->frames.push_back(frame);
            jsonFillColor(jsonFrame, "clear", frame->clear);
            jsonFillInt(jsonFrame, "fadeIn", frame->fadeIn);
            jsonFillInt(jsonFrame, "duration", frame->duration);
            jsonFillInt(jsonFrame, "fadeOut", frame->fadeOut);
            cJSON * jsonRenders = cJSON_GetObjectItem(jsonFrame, "renders");
            if (jsonRenders) {
                assert(cJSON_IsArray(jsonRenders));
                for (cJSON * jsonRender = jsonRenders->child; jsonRender; jsonRender = jsonRender->next) {
                    Render * render = new Render;
                    frame->renders.push_back(render);
                    jsonFillString(jsonRender, "src", render->src);
                    jsonFillColor(jsonRender, "color", render->color);
                    jsonFillFloat(jsonRender, "x", render->x);
                    jsonFillFloat(jsonRender, "y", render->y);
                    jsonFillFloat(jsonRender, "w", render->w);
                    jsonFillFloat(jsonRender, "h", render->h);
                    jsonFillFloat(jsonRender, "ax", render->ax);
                    jsonFillFloat(jsonRender, "ay", render->ay);
                    jsonFillFloat(jsonRender, "r", render->r);
                    jsonFillInt(jsonRender, "durationAnim", render->durationAnim);
                    jsonFillFloat(jsonRender, "xAnim", render->xAnim);
                    jsonFillFloat(jsonRender, "yAnim", render->yAnim);
                    jsonFillFloat(jsonRender, "wAnim", render->wAnim);
                    jsonFillFloat(jsonRender, "hAnim", render->hAnim);
                    jsonFillFloat(jsonRender, "axAnim", render->axAnim);
                    jsonFillFloat(jsonRender, "ayAnim", render->ayAnim);
                    jsonFillFloat(jsonRender, "rAnim", render->rAnim);
                }
            }
            cJSON * jsonDialogue = cJSON_GetObjectItem(jsonFrame, "dialogue");
            if (jsonDialogue) {
                assert(cJSON_IsArray(jsonDialogue));
                for (cJSON * jsonLine = jsonDialogue->child; jsonLine; jsonLine = jsonLine->next) {
                    assert(cJSON_IsString(jsonLine));
                    frame->dialogue.push_back(jsonLine->valuestring);
                }
            }
            jsonFillColor(jsonFrame, "dialogueTextColor", frame->dialogueTextColor);
            jsonFillFloat(jsonFrame, "dialogueTextSize", frame->dialogueTextSize);
            jsonFillInt(jsonFrame, "dialogueSpeed", frame->dialogueSpeed);
            jsonFillString(jsonFrame, "centerText", frame->centerText);
            jsonFillColor(jsonFrame, "centerTextColor", frame->centerTextColor);
            jsonFillFloat(jsonFrame, "centerTextSize", frame->centerTextSize);
            jsonFillString(jsonFrame, "bgm", frame->bgm);
        }
    }
    cJSON_Delete(json);
    return true;
}

void Cutscene::startScene(const char * sceneName)
{
    std::map<std::string, Scene *>::iterator it = scenes_.find(sceneName);
    assert(it != scenes_.end());
    currentScene_ = it->second;
    bgm_ = -1;
    frameIndex_ = -1;
#if _DEBUG
    // Tune here to debug cutscenes
    frameIndex_ = -1;
#endif
    nextFrame();
}

bool Cutscene::end()
{
    if (bgm_ != -1) {
        sound::stop(bgm_);
    }
    return false;
}

bool Cutscene::update()
{
    if (!currentScene_)
        return end();

    if (input::pressed(input::START)) {
        return end();
    }

    Frame * frame = currentScene_->frames[frameIndex_];
    switch (state_) {
        case STATE_FADEIN:
            if (stateElapsedMS() >= frame->fadeIn) {
                if (frame->dialogue.size() > 0) {
                    switchState(STATE_TYPING);
                } else {
                    switchState(STATE_WAIT);
                }
            }
            break;
        case STATE_WAIT:
        {
            bool done = false;
            if (frame->duration > 0) {
                done = (stateElapsedMS() >= frame->duration);
            }
            if (input::pressed(input::ACCEPT)) {
                done = true;
            }
            if (done) {
                if (frame->fadeOut) {
                    switchState(STATE_FADEOUT);
                } else {
                    return nextFrame();
                }
            }
            break;
        }
        case STATE_TYPING:
        {
            int totalChars = 0;
            for (std::vector<std::string>::iterator it = frame->dialogue.begin(); it != frame->dialogue.end(); ++it) {
                totalChars += (int)it->size();
            }
            int typedChars = (int)(stateElapsedMS() / frame->dialogueSpeed);
            bool done = false;
            if (typedChars >= totalChars) {
                done = true;
            }
            if (input::pressed(input::ACCEPT)) {
                done = true;
            }
            if (done) {
                switchState(STATE_WAIT);
            }
            break;
        }
        case STATE_FADEOUT:
            if (stateElapsedMS() >= frame->fadeOut)
                return nextFrame();
            break;
    }
    return true;
}

void Cutscene::render()
{
    if (!currentScene_)
        return;

    Frame * frame = currentScene_->frames[frameIndex_];

    gfx::draw(0, 0, os::winWf(), os::winHf(), nullptr, &frame->clear);

    for (std::vector<Render *>::iterator rit = frame->renders.begin(); rit != frame->renders.end(); ++rit) {
        Render * r = *rit;
        if ((r->w == 0) && (r->h == 0))
            continue;

        gfx::DrawSource * src = nullptr;
        gfx::DrawSource found;
        if (r->src.size() > 0) {
            if (spritesheet_.findSprite(r->src, found)) {
                src = &found;
            }
        }

        if ((src == nullptr) && ((r->w == 0) || (r->h == 0))) {
            // Fills need a width and a height
            continue;
        }

        float rx = r->x;
        float ry = r->y;
        float rw = r->w;
        float rh = r->h;
        float rax = r->ax;
        float ray = r->ay;
        float rr = r->r;

        if (r->durationAnim > 0) {
            float p = os::clamp((float)frameElapsedMS() / (float)r->durationAnim, 0.0f, 1.0f);
            rx += r->xAnim * p;
            ry += r->yAnim * p;
            rw += r->wAnim * p;
            rh += r->hAnim * p;
            rax += r->axAnim * p;
            ray += r->ayAnim * p;
            rr += r->rAnim * p;
        }

        float x = rx * os::winWf();
        float y = ry * os::winHf();
        float w, h;
        if (rw == 0.0f) {
            h = rh * os::winHf();
            w = h * src->w / src->h;
        } else if (rh == 0.0f) {
            w = rw * os::winWf();
            h = w * src->h / src->w;
        } else {
            w = rw * os::winWf();
            h = rh * os::winHf();
        }
        gfx::draw(x, y, w, h, src, &r->color, rax, ray, rr);
    }

    if (frame->dialogue.size() > 0) {
        float dialogueTop = os::winHf() * (1.0f - frame->dialogueSlice);

        // Draw the dialogue frame underneath
        gfx::Color black = { 0, 0, 0, 255 };
        gfx::draw(0, dialogueTop, os::winWf(), os::winHf() - dialogueTop, nullptr, &black);

        int charsRemaining = (int)(stateElapsedMS() / frame->dialogueSpeed);
        if (state_ < STATE_TYPING) {
            charsRemaining = 0;
        }
        if (state_ > STATE_TYPING) {
            charsRemaining = 0x7fffffff;
        }

        float fontSize = frame->dialogueTextSize * os::winHf();
        std::string line;
        int lineIndex = 0;
        for (std::vector<std::string>::iterator it = frame->dialogue.begin(); it != frame->dialogue.end(); ++it) {
            ++lineIndex;
            line = *it;
            if (line.size() > charsRemaining) {
                line.resize(charsRemaining);
            }
            float x = os::winWf() * 0.1f;
            float y = dialogueTop + (fontSize * lineIndex);
            gfx::drawText(x, y, line.c_str(), font_, fontSize, &frame->dialogueTextColor, 0.0f, 0.0f);

            charsRemaining -= (int)line.size();
            if (charsRemaining <= 0)
                break;
        }
    }

    if (frame->centerText.size() > 0) {
        float fontSize = frame->centerTextSize * os::winHf();
        float opacity = 1.0f;
        gfx::drawText(os::winWf() / 2, os::winHf() / 2, frame->centerText.c_str(), font_, fontSize, &frame->centerTextColor);
    }

    if ((state_ == STATE_FADEIN) || (state_ == STATE_FADEOUT)) {
        float opacity;
        if (state_ == STATE_FADEIN) {
            opacity = 1.0f - os::clamp((float)stateElapsedMS() / (float)frame->fadeIn, 0.0f, 1.0f);
        } else {
            opacity = os::clamp((float)stateElapsedMS() / (float)frame->fadeOut, 0.0f, 1.0f);
        }
        gfx::Color black = { 0, 0, 0, (unsigned char)(255 * opacity) };
        gfx::draw(0, 0, os::winWf(), os::winHf(), nullptr, &black);
    }
}
