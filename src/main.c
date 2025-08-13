#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/togglemutedaemon"
#define BUFFER_SIZE 1

int muted = 0;

void toggle_mute()
{
    pid_t pid = fork();
    if (pid == -1) {
        perror("failed to fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // wpctl set-mute @DEFAULT_AUDIO_SOURCE@ 1
        if (muted == 0) {
            printf("Muting\n");
            execlp("wpctl", "wpctl", "set-mute", "@DEFAULT_AUDIO_SOURCE@", "1", NULL);
        } else {
            printf("Unmuting\n");
            execlp("wpctl", "wpctl", "set-mute", "@DEFAULT_AUDIO_SOURCE@", "0", NULL);
        }
    } else {
        muted = !muted;
    }
}

int server()
{
    struct sockaddr_un server_addr;
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("SERVER: failed to create server socket");
        exit(EXIT_FAILURE);
    }

    unlink(SOCKET_PATH);

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_PATH);
    int len = sizeof(server_addr);

    int rc = bind(server_fd, (struct sockaddr*)&server_addr, len);
    if (rc == -1) {
        perror("SERVER: failed to bind server socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    rc = listen(server_fd, 2);
    if (rc == -1) {
        perror("SERVER: listen error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("SERVER: Socket listening\n");
        struct sockaddr_un client_addr;
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&len);
        if (client_fd == -1) {
            perror("SERVER: failed to accept");
            close(server_fd);
            close(client_fd);
            exit(EXIT_FAILURE);
        }

        char buf[BUFFER_SIZE] = { 0 };
        int byte_recv = recv(client_fd, buf, sizeof(buf), 0);
        if (byte_recv == -1) {
            perror("SERVER: Failed to recv");
            close(server_fd);
            close(client_fd);
            exit(EXIT_FAILURE);
        }

        printf("received this \"%s\"\n", buf);
        close(client_fd);

        if (buf[0] == 't') {
            toggle_mute();
        }

        if (buf[0] == 'q') {
            goto cleanup;
        }
    }

cleanup:
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}

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
    printf("Client connected to server.\n");

    if (write(client_fd, &msg, sizeof(msg)) == -1) {
        perror("client: write");
    }
    printf("Client sent: '%c'\n", msg);

    close(client_fd);
    return 0;
}

void usage(char* name) {
    printf("Usage: %s\ns - server/daemon\nc - client; t(toggle mute), q(exit)\n", name);
}

int main(int argc, char** argv)
{
    if (argc == 2) {
        if (strcmp(argv[1], "s") == 0) {
            server();
        } else {
            usage(argv[0]); 
        }
    } else if (argc == 3) {
        if (strcmp(argv[1], "c") == 0) {
            client(argv[2][0]);
        } else {
            usage(argv[0]); 
        }
    } else {
        usage(argv[0]);
    }

    return 0;
}
