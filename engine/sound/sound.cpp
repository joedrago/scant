#include "sound/sound.h"

#include "os/os.h"

#include <assert.h>

#define TS_IMPLEMENTATION
#include "stb_vorbis.h"
#include "tinysound.h"

// #define USE_MIX_THREAD

namespace sound
{

static tsContext * context_;

struct SoundRecord
{
    std::string path;
    bool looping;
    int maxActive;
    tsLoadedSound *loadedSound;
    std::vector<tsPlayingSound *> active;
};
static std::vector<SoundRecord> sounds_;

void startup()
{
    context_ = tsMakeContext(os::windowHandle(), 44100, 5, 5, 0);
#if defined(USE_MIX_THREAD)
    tsSpawnMixThread(context_);
#endif
}

void shutdown()
{
    tsShutdownContext(context_);
    // TODO: delete sounds_ properly
}

void update()
{
#if !defined(USE_MIX_THREAD)
    tsMix(context_);
#endif

    for (std::vector<SoundRecord>::iterator soundIt = sounds_.begin(); soundIt != sounds_.end(); ++soundIt) {
        SoundRecord & sr = *soundIt;

        std::vector<tsPlayingSound *>::iterator activeIt = sr.active.begin();
        while (activeIt != sr.active.end()) {
            tsPlayingSound * playingSound = *activeIt;
            if (!tsIsActive(playingSound) && !playingSound->inserted) {
                delete playingSound;
                activeIt = sr.active.erase(activeIt);
            } else {
                ++activeIt;
            }
        }
    }
}

static int findSoundRecord(const char *path)
{
    int soundCount = (int)sounds_.size();
    for(int i = 0; i < soundCount; ++i) {
        if(sounds_[i].path == path) {
            return i;
        }
    }
    return -1;
}

static int addSoundRecord(int maxActive, bool looping, const char *path)
{
    int id = (int)sounds_.size();
    sounds_.resize(sounds_.size() + 1);
    SoundRecord & sr = sounds_[id];
    sr.path = path;
    sr.loadedSound = new tsLoadedSound;
    sr.maxActive = maxActive;
    sr.looping = looping;
    return id;
}

int loadWAV(const char * filename, int maxActive, bool looping)
{
    int id = findSoundRecord(filename);
    if(id != -1)
        return id;

    id = addSoundRecord(maxActive, looping, filename);
    SoundRecord & sr = sounds_[id];
    *sr.loadedSound = tsLoadWAV(filename);
    return id;
}

int loadOGG(const char * filename, int maxActive, bool looping)
{
    int id = findSoundRecord(filename);
    if(id != -1)
        return id;

    id = addSoundRecord(maxActive, looping, filename);
    SoundRecord & sr = sounds_[id];
    int sampleRate = 44100;
    *sr.loadedSound = tsLoadOGG(filename, &sampleRate);
    return id;
}

void play(int id)
{
    assert((id >= 0) && (id < sounds_.size()));

    SoundRecord & sr = sounds_[id];
    if ((int)sr.active.size() >= sr.maxActive) {
        int toDeactivate = 1 + (int)sr.active.size() - sr.maxActive;
        for (int i = 0; i < toDeactivate; ++i) {
            tsStopSound(sr.active[i]);
        }
    }

    tsPlayingSound * playingSound = new tsPlayingSound;
    *playingSound = tsMakePlayingSound(sr.loadedSound);
    tsLoopSound(playingSound, sr.looping ? 1 : 0);
    sr.active.push_back(playingSound);
    tsInsertSound(context_, playingSound);
}

void stop(int id)
{
    assert((id >= 0) && (id < sounds_.size()));

    SoundRecord & sr = sounds_[id];
    for(std::vector<tsPlayingSound *>::iterator activeIt = sr.active.begin(); activeIt != sr.active.end(); ++activeIt) {
        tsStopSound(*activeIt);
    }
}

void stopAll()
{
    tsStopAllSounds(context_);
}

}
