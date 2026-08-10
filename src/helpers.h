#pragma once

#include "../external/miniaudio.h"

// Expects a char buffer of PATH_MAX length
char* executable_path(char* const buffer);

int init_audio(ma_engine* engine);
void deinit_audio(ma_engine* engine);
void play_sound(ma_engine* engine, const char* filepath);

size_t get_time_ms();
void sleep_ms(size_t milliseconds);
