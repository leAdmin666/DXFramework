#include <stdio.h>

#include "components/audio_player.h"

int main() {
    AlsaPlayer player;

    AlsaPlayer_init(&player, 44100, 2, 32);
    
    if(!AlsaPlayer_initialize(&player)) {
        fprintf(stderr, "Failed to initialize AlsaPlayer\n");
        return 1;
    }

    AlsaPlayer_playWavFile(&player, "test.wav");
    AlsaPlayer_playWavFile(&player, "test.wav");
    AlsaPlayer_destroy(&player);

    return 0;
}