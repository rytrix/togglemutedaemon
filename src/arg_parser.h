#pragma once

enum ArgValues {
    ARG_NOT_PRESENT = 0,
    ARG_PRESENT = 1,
};
typedef enum ArgValues ArgValues_t;

struct Args {
    enum ArgValues server;
    enum ArgValues client;
    enum ArgValues audio;
    enum ArgValues push_to_talk;
    enum ArgValues global_ptt;
    const char* message;
};
typedef struct Args Args_t;

Args_t args_default();
Args_t parse_args(int argc, char** argv);
void usage(char* name);
