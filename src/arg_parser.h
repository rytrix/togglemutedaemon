#pragma once

typedef enum {
    ARG_NOT_PRESENT = 0,
    ARG_PRESENT = 1,
} arg_values_t;

typedef struct args args_t;
struct args {
    arg_values_t server;
    arg_values_t client;
    arg_values_t audio;
    arg_values_t push_to_talk;
    arg_values_t global_ptt;
    const char* message;
};

args_t args_default();
args_t parse_args(int argc, char** argv);
void usage(char* name);
