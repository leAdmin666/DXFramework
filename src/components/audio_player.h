#pragma once

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../audio_headers/riff_wave_header.h"

typedef struct {
    unsigned int rate;
    int channels;
    snd_pcm_uframes_t frames;
    snd_pcm_uframes_t buffer_size;
    snd_pcm_t* playback_handle;
    snd_pcm_hw_params_t* hw_params;
} AlsaPlayer;

void AlsaPlayer_destroy(AlsaPlayer* player) {
    if (player->playback_handle) {
        snd_pcm_drain(player->playback_handle);
        snd_pcm_close(player->playback_handle);
    }
    if (player->hw_params) {
        snd_pcm_hw_params_free(player->hw_params);
    }
}

int AlsaPlayer_init(AlsaPlayer* player, unsigned int rate, int channels, snd_pcm_uframes_t frames) {
    player->rate = rate;
    player->channels = channels;
    player->frames = frames;
    player->buffer_size = frames * 4;
    player->playback_handle = NULL;
    player->hw_params = NULL;

    int err;

    if ((err = snd_pcm_open(&player->playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("Cannot open audio device: %s\n", snd_strerror(err));
        return 0;
    }

    if ((err = snd_pcm_hw_params_malloc(&player->hw_params)) < 0) {
        printf("Cannot allocate hardware parameter structure: %s\n", snd_strerror(err));
        return 0;
    }

    if ((err = snd_pcm_hw_params_any(player->playback_handle, player->hw_params)) < 0) {
        printf("Cannot initialize hardware parameter structure: %s\n", snd_strerror(err));
        return 0;
    }

    if ((err = snd_pcm_hw_params_set_access(player->playback_handle, player->hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        printf("Cannot set access type: %s\n", snd_strerror(err));
        return 0;
    }

    if ((err = snd_pcm_hw_params_set_format(player->playback_handle, player->hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        printf("Cannot set sample format: %s\n", snd_strerror(err));
        return 0;
    }

    if ((err = snd_pcm_hw_params_set_rate_near(player->playback_handle, player->hw_params, &rate, 0)) < 0) {
        printf("Cannot set sample rate: %s\n", snd_strerror(err));
        return 0;
    }

    if ((err = snd_pcm_hw_params_set_channels(player->playback_handle, player->hw_params, channels)) < 0) {
        printf("Cannot set channel count: %s\n", snd_strerror(err));
        return 0;
    }

    if ((err = snd_pcm_hw_params_set_buffer_size_near(player->playback_handle, player->hw_params, &player->buffer_size)) < 0) {
        printf("Cannot set buffer size: %s\n", snd_strerror(err));
        return 0;
    }

    if ((err = snd_pcm_hw_params(player->playback_handle, player->hw_params)) < 0) {
        printf("Cannot set parameters: %s\n", snd_strerror(err));
        return 0;
    }

    snd_pcm_hw_params_free(player->hw_params);
    player->hw_params = NULL;

    if ((err = snd_pcm_prepare(player->playback_handle)) < 0) {
        printf("Cannot prepare audio interface for use: %s\n", snd_strerror(err));
        return 0;
    }

    return 1;
}

void AlsaPlayer_playSineWave(AlsaPlayer* player, float frequency, int duration) {
    int err;
    int16_t buffer[player->frames * player->channels];
    for (int i = 0; i < player->frames * player->channels; i += 2) {
        buffer[i] = buffer[i + 1] = 32767 * sin((2 * M_PI * frequency * (i / 2)) / player->rate);
    }

    for (int i = 0; i < duration; ++i) {
        if ((err = snd_pcm_writei(player->playback_handle, buffer, player->frames)) != player->frames) {
            if (err == -EPIPE) {
                printf("Buffer underrun occurred\n");
                snd_pcm_prepare(player->playback_handle);
            } else if (err < 0) {
                printf("Write to audio interface failed: %s\n", snd_strerror(err));
                return;
            }
        }
    }
}

void AlsaPlayer_playWavFile(AlsaPlayer* player, const char* file_path) {
    WavFile wavFile;
    if(!load_wav_file(&wavFile, file_path)) {
        printf("Failed to load WAV file: %s\n", file_path);
        return;
    }

    if (!AlsaPlayer_init(player, wavFile.header.sample_rate, wavFile.header.num_channels, player->frames)) {
        printf("Failed to reinitialize ALSA player with WAV file parameters\n");
        return;
    }

    int err;
    snd_pcm_prepare(player->playback_handle);
    size_t index = 0;
    size_t total_samples = wavFile.header.data_size / (wavFile.header.bits_per_sample / 8);

    while (index < total_samples) {
        size_t samples_remaining = total_samples - index;
        size_t frames_to_write = (player->frames < samples_remaining / wavFile.header.num_channels) ? player->frames : samples_remaining / wavFile.header.num_channels;
        if ((err = snd_pcm_writei(player->playback_handle, wavFile.data + index, frames_to_write)) != frames_to_write) {
            if (err == -EPIPE) {
                printf("Buffer underrun occurred\n");
                snd_pcm_prepare(player->playback_handle);
            } else if (err < 0) {
                printf("Write to audio interface failed: %s\n", snd_strerror(err));
                return;
            }
        }
        index += frames_to_write * wavFile.header.num_channels;
    }

    // Write remaining silence to ensure playback completes without noise
    size_t remaining_frames = player->frames - (wavFile.header.data_size % player->frames);
    int16_t* silence = (int16_t*)calloc(remaining_frames * player->channels, sizeof(int16_t));
    if(!silence) {
        printf("Error allocating memory for silence buffer\n");
        free_wav_file(&wavFile);
        return;
    }

    if ((err = snd_pcm_writei(player->playback_handle, silence, remaining_frames)) != remaining_frames) {
        if (err == -EPIPE) {
            printf("Buffer underrun occurred during silence fill\n");
            snd_pcm_prepare(player->playback_handle);
        } else if (err < 0) {
            printf("Write to audio interface failed during silence fill: %s\n", snd_strerror(err));
            return;
        }
    }

    free(silence);
    snd_pcm_drain(player->playback_handle);
    free_wav_file(&wavFile);
}