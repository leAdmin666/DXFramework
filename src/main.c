#include <stdio.h>

#include "components/audio_player.h"

int main() {
    AlsaPlayer player;
    
    if(!AlsaPlayer_init(&player, 44100, 2, 32)) {
        fprintf(stderr, "Failed to initialize AlsaPlayer\n");
        return 1;
    }

    AlsaPlayer_playWavFile(&player, "test.wav");
    AlsaPlayer_playWavFile(&player, "test.wav");
    AlsaPlayer_destroy(&player);

    return 0;
}