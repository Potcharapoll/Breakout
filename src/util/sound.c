#include "sound.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <audio/wave.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

static inline ALenum get_al_format(int8_t channels, int8_t samples) {
    bool stereo = (channels > 1);

    switch(samples) {
        case 16:
            if (stereo)
                return AL_FORMAT_STEREO16;
            else 
                return AL_FORMAT_MONO16;
        case 8:
            if (stereo)
                return AL_FORMAT_STEREO8;
            else 
                return AL_FORMAT_MONO8;
        default:
            return -1;
    }
}

void sound_context_init(sound_context_t *self) {
    self->device  = alcOpenDevice(NULL);
    assert(self->device != NULL);

    self->context = alcCreateContext(self->device, NULL);
    assert(self->context != NULL);
}

void sound_context_use(sound_context_t *self) {
    alcMakeContextCurrent(self->context);
}

void sound_context_destroy(sound_context_t *self) {
    alcMakeContextCurrent(NULL);
    alcDestroyContext(self->context);
    alcCloseDevice(self->device);
}

sound_t* sound_init(const char *file_path) {
    sound_t *sound = malloc(sizeof(*sound));

    sound->wave = WaveOpenFileForReading(file_path);
    assert(sound->wave != NULL);

    int ret = WaveSeekFile(0, sound->wave);
    assert(ret == 0);

    char *data = malloc(sound->wave->dataSize);
    assert(data != NULL);

    WaveReadFile(data, sound->wave->dataSize, sound->wave);

    alGenBuffers(1, &sound->buffer);
    alGenSources(1, &sound->source);

    alBufferData(
            sound->buffer, 
            get_al_format(sound->wave->channels, sound->wave->bitsPerSample),
            data, 
            sound->wave->dataSize, 
            sound->wave->sampleRate);
    free(data);

    // listener
    alListener3f(AL_POSITION, 0.0f, 0.0f, 1.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);

    // source
    alSourcef(sound->source,  AL_PITCH,    1.0f);
    alSourcef(sound->source,  AL_GAIN,     1.0f);
    alSource3f(sound->source, AL_POSITION, 0.0f, 0.0f,0.0f);
    alSource3f(sound->source, AL_VELOCITY, 0.0f, 0.0f,0.0f);
    alSourcei(sound->source,  AL_LOOPING,  AL_FALSE);
    alSourcei(sound->source,  AL_BUFFER,   sound->buffer);
    return sound;
}

void sound_play(sound_t *self) {
    alSourcePlay(self->source);
}

void sound_free(sound_t *self) {
    WaveCloseFile(self->wave);
    alDeleteSources(1, &self->source);
    alDeleteBuffers(1, &self->buffer);
    free(self);
}

