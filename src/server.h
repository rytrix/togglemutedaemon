#include "../external/miniaudio.h"

#include <stdatomic.h>

enum MicStatus {
    STATUS_MUTED = 0,
    STATUS_UNMUTED = 1,
};

struct ServerContext {
    enum MicStatus mic_status;
    atomic_size_t prev_ptt_ms;
    atomic_int ptt_worker_continue;
    pthread_mutex_t mutex;

    ma_engine engine;
};

int server(int ptt, int play_sound);

