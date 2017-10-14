#ifndef SOUND_SOUND_H
#define SOUND_SOUND_H

namespace sound
{

void startup();
void shutdown();
void update();

int loadWAV(const char * filename, int maxActive = 1, bool looping = false);
int loadOGG(const char * filename, int maxActive = 1, bool looping = false);
void play(int id);
void stop(int id);
void stopAll();

}

#endif
