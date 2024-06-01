#include <stdio.h>

#include "components/audio_player.h"

int main() {
    AlsaPlayer player;
    AudioQueue queue;

    AudioQueue_init(&queue);

    AudioQueue_enqueue(&queue, "test.wav");
    
    if(!AlsaPlayer_init(&player, 44100, 2, 32)) {
        fprintf(stderr, "Failed to initialize AlsaPlayer\n");
        return 1;
    }

    AlsaPlayer_playQueue(&player, &queue);

    AudioQueue_destroy(&queue);
    AlsaPlayer_destroy(&player);

    return 0;
}