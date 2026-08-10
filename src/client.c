#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "debug.h"
#include "globals.h"

int client(char msg)
{
    int client_fd;
    struct sockaddr_un server_addr;

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("client: socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("client: connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    printf_debug("Client connected to server.\n");

    if (write(client_fd, &msg, sizeof(msg)) == -1) {
        perror("client: write");
    }
    printf_debug("Client sent: '%c'\n", msg);

    close(client_fd);
    return 0;
}
