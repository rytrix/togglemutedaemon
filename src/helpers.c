#include "helpers.h"

#include "globals.h"

#include <libgen.h>
#include <stdio.h>
#include <unistd.h>

// Expects a char buffer of PATH_MAX length
char* executable_path(char* const buffer)
{
    size_t len = readlink("/proc/self/exe", buffer, PATH_MAX - 1);
    if (len == -1) {
        perror("Readlink failed, unable to determine executable directory");
        return buffer;
    }

    buffer[len] = '\0';
    return dirname(buffer);
}

int init_audio(ma_engine* engine)
{
    ma_result result;
    result = ma_engine_init(NULL, engine);
    if (result != MA_SUCCESS) {
        perror("Failed to start audio engine");
        return result; // Failed to initialize the engine.
    }
    return 0;
}

void deinit_audio(ma_engine* engine)
{
    ma_engine_uninit(engine);
}

void play_sound(ma_engine* engine, const char* filepath)
{
    ma_engine_play_sound(engine, filepath, NULL);
}

size_t get_time_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (size_t)(ts.tv_sec * 1000) + (size_t)(ts.tv_nsec / 1000000);
}

void sleep_ms(size_t milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}
