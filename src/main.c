#include <stdio.h>

#include "arg_parser.h"
#include "client.h"
#include "debug.h"
#include "server.h"
#include "watch_key.h"

int main(int argc, char** argv)
{
    Args_t args = parse_args(argc, argv);
    if (args.server == ARG_PRESENT) {
        int audio = 0;
        int ptt = 0;
        if (args.audio == ARG_PRESENT) {
            audio = 1;
        }
        if (args.push_to_talk) {
            ptt = 1;
        }
        server(ptt, audio);
    } else if (args.client == ARG_PRESENT) {
        if (args.message == NULL) {
            usage(argv[0]);
        }
        client(*args.message);
    } else if (args.global_ptt == ARG_PRESENT) {
        watch_key(args.message);
    } else {
        usage(argv[0]);
    }

    return 0;
}
