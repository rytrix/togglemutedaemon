#include "arg_parser.h"

#include "debug.h"

#include <string.h>
#include <stdio.h>

void usage(char* name)
{
    printf("Usage: %s\ns - server/daemon; a(audio), p(push to talk)\nc - client; t(toggle mute), 0(unmute), 1(mute) q(exit)\ng - global push to talk\n", name);
}

Args_t args_default()
{
    Args_t args;
    args.server = ARG_NOT_PRESENT;
    args.client = ARG_NOT_PRESENT;
    args.global_ptt = ARG_NOT_PRESENT;
    args.audio = ARG_NOT_PRESENT;
    args.push_to_talk = ARG_NOT_PRESENT;
    args.message = NULL;
    return args;
}

Args_t parse_args(int argc, char** argv)
{
    Args_t args = args_default();
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "s") == 0) {
            args.server = ARG_PRESENT;
        }
        if (strcmp(argv[i], "c") == 0) {
            args.client = ARG_PRESENT;
        }
        if (strcmp(argv[i], "g") == 0) {
            args.global_ptt = ARG_PRESENT;
        }
        if (strcmp(argv[i], "p") == 0) {
            args.push_to_talk = ARG_PRESENT;
        }
        if (strcmp(argv[i], "a") == 0) {
            args.audio = ARG_PRESENT;
        }
        if (strcmp(argv[i], "t") == 0) {
            if (args.message != NULL) {
                printf_debug("Warning: message \"%s\" already present, overwriting with \"%s\"\n", args.message, argv[i]);
            }
            args.message = argv[i];
        }
        if (strcmp(argv[i], "0") == 0) {
            if (args.message != NULL) {
                printf_debug("Warning: message \"%s\" already present, overwriting with \"%s\"\n", args.message, argv[i]);
            }
            args.message = argv[i];
        }
        if (strcmp(argv[i], "1") == 0) {
            if (args.message != NULL) {
                printf_debug("Warning: message \"%s\" already present, overwriting with \"%s\"\n", args.message, argv[i]);
            }
            args.message = argv[i];
        }
    }

    return args;
}

