#ifndef AUDIO_H
#define AUDIO_H

#include "base.h"

typedef struct audio audio_o;

void audio_init();
void audio_close();

audio_o *audio_load(string_t path);
void audio_delete(audio_o *audio);

void audio_play(audio_o *audio);
void audio_pause(audio_o *audio);
void audio_stop(audio_o *audio);

bool8_t audio_playing(audio_o *audio);
void audio_volume_set(audio_o *audio, float32_t volume);
float32_t audio_volume_get(audio_o *audio);

void audio_gobal_volume_set(float32_t volume);
float32_t audio_gobal_volume_get();

#endif // AUDIO_H