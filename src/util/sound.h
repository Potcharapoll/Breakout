#ifndef SOUND_H
#define SOUND_H

#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>
#include <audio/wave.h>

typedef struct {
    ALCdevice *device;
    ALCcontext *context;
} sound_context_t;

typedef struct {
    WaveInfo *wave;
    ALuint buffer;
    ALuint source;
} sound_t;

void sound_context_init(sound_context_t *self);
void sound_context_destroy(sound_context_t *self);
void sound_context_use(sound_context_t *self);

sound_t* sound_init(const char *file_path);
void sound_play(sound_t *self);
void sound_free(sound_t *self);
#endif
