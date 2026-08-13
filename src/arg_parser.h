#pragma once

enum arg_values {
    ARG_NOT_PRESENT = 0,
    ARG_PRESENT = 1,
};
typedef enum arg_values arg_values_t;

struct args {
    enum arg_values server;
    enum arg_values client;
    enum arg_values audio;
    enum arg_values push_to_talk;
    enum arg_values global_ptt;
    const char* message;
};
typedef struct args args_t;

args_t args_default();
args_t parse_args(int argc, char** argv);
void usage(char* name);
