#include "../external/miniaudio.h"
#include <libgen.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/togglemutedaemon"
#define BUFFER_SIZE 1

#ifdef DEBUG
#define printf_debug printf
#else
#define printf_debug //
#endif

int muted = 0;

ma_engine engine;

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

int init_audio()
{
    ma_result result;
    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        perror("Failed to start audio engine");
        return result; // Failed to initialize the engine.
    }
    return 0;
}

void deinit_audio()
{
    ma_engine_uninit(&engine);
}

void play_sound(const char* filepath)
{
    ma_engine_play_sound(&engine, filepath, NULL);
}

void toggle_mute(char* sound_dir)
{
    pid_t pid = fork();
    if (pid == -1) {
        perror("failed to fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // wpctl set-mute @DEFAULT_AUDIO_SOURCE@ 1
        if (muted == 0) {
            printf_debug("Muting\n");
            execlp("wpctl", "wpctl", "set-mute", "@DEFAULT_AUDIO_SOURCE@", "1", NULL);
        } else {
            printf_debug("Unmuting\n");
            execlp("wpctl", "wpctl", "set-mute", "@DEFAULT_AUDIO_SOURCE@", "0", NULL);
        }
    } else {
        char sound_path[PATH_MAX + 12] = { 0 };
        strcpy(sound_path, sound_dir);
        int path_len = strlen(sound_path);
        if (muted) {
            strcpy(sound_path + path_len, "/unmuted.mp3\0");
            printf_debug("Attempting to play sound: \"%s\"\n", sound_path);
            play_sound(sound_path);
        } else {
            strcpy(sound_path + path_len, "/muted.mp3\0");
            printf_debug("Attempting to play sound: \"%s\"\n", sound_path);
            play_sound(sound_path);
        }
        muted = !muted;
    }
}

int server()
{
    char executable_path_buffer[PATH_MAX];
    char* sounds_dir = executable_path(executable_path_buffer);
    int path_len = strlen(sounds_dir);
    const char* sounds = "/sounds\0";
    strcpy(sounds_dir + path_len, sounds);
    printf_debug("SERVER: Sound path: \"%s\"\n", sounds_dir);

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
        printf_debug("SERVER: Socket listening\n");
        struct sockaddr_un client_addr;
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&len);
        if (client_fd == -1) {
            perror("SERVER: failed to accept");
            close(server_fd);
            close(client_fd);
            exit(EXIT_FAILURE);
        }

        char buf[BUFFER_SIZE + 1] = { 0 };
        int byte_recv = recv(client_fd, buf, BUFFER_SIZE, 0);
        if (byte_recv == -1) {
            perror("SERVER: Failed to recv");
            close(server_fd);
            close(client_fd);
            exit(EXIT_FAILURE);
        }

        printf_debug("SERVER: received \"%s\"\n", buf);
        close(client_fd);

        if (buf[0] == 't') {
            toggle_mute(sounds_dir);
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
    struct sockaddr_un server_addr = { 0 };

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("client: socket");
        exit(EXIT_FAILURE);
    }

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

void usage(char* name)
{
    printf("Usage: %s\ns - server/daemon\nc - client; t(toggle mute), q(exit)\n", name);
}

int main(int argc, char** argv)
{
    if (argc == 2) {
        if (strcmp(argv[1], "s") == 0) {
            int audio_result = init_audio();
            if (audio_result != 0) {
                return audio_result;
            }
            server();
            deinit_audio();
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
