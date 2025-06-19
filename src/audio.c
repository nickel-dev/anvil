#include "base.h"
#include "core.h"
#include "audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

struct audio {
    ma_sound sound;
    bool8_t playing;
};

global ma_result result;
global ma_engine engine;

void audio_init() {
    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        os_message(OS_MESSAGE_ERROR, "Failed to initialize audio engine\nError core: %i", result);
        exit(EXIT_FAILURE);
    }
}

void audio_close() {
    ma_engine_uninit(&engine);
}

audio_o *audio_load(string_t path) {
    audio_o *audio = malloc(sizeof(audio_o));
	if (!audio) {
		os_message(OS_MESSAGE_ERROR, "Failed to allocate memory for audio\nPath: %s", path);
		exit(EXIT_FAILURE);
	}
	
	ZERO_MEMORY(audio);
    result = ma_sound_init_from_file(&engine, path, 0, NULL, NULL, &audio->sound);
	
    if (result != MA_SUCCESS) {
		os_message(OS_MESSAGE_ERROR, "Failed to load audio\nPath: %s", path);
        exit(EXIT_FAILURE);
    }
	
    return audio;
}

void audio_delete(audio_o *audio) {
	ma_sound_uninit(&audio->sound);
	if (audio) {
		free(audio);
	}
}

void audio_play(audio_o *audio) {
	if (!ma_sound_is_playing(&audio->sound)) {
		audio->playing = false;
	}
	
	if (!audio->playing) {
		ma_sound_start(&audio->sound);
		audio->playing = true;
	}
}

void audio_pause(audio_o *audio) {
	if (audio->playing) {
		ma_sound_stop(&audio->sound);
		audio->playing = false;
	}
}

void audio_stop(audio_o *audio) {
	if (audio->playing) {
		ma_sound_stop(&audio->sound);
		ma_sound_seek_to_pcm_frame(&audio->sound, 0);
		audio->playing = false;
	}
}

bool8_t audio_playing(audio_o *audio) {
    return audio->playing;
}

void audio_volume_set(audio_o *audio, float32_t volume) {
    ma_sound_set_volume(&audio->sound, volume);
}

float32_t audio_volume_get(audio_o *audio) {
    return ma_sound_get_volume(&audio->sound);
}

void audio_gobal_volume_set(float32_t volume) {
    ma_sound_set_volume((ma_sound *)&engine, volume);
}

float32_t audio_gobal_volume_get() {
    return ma_sound_get_volume((ma_sound *)&engine);
}