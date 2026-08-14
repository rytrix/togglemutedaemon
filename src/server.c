#include "server.h"

#include "debug.h"
#include "globals.h"
#include "helpers.h"

#include <libgen.h>
#include <linux/limits.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "../external/miniaudio.h"

enum mic_status {
    STATUS_MUTED = 0,
    STATUS_UNMUTED = 1,
};
typedef enum mic_status mic_status_t;

typedef struct server_context server_context_t;
struct server_context {
    mic_status_t mic_status;
    atomic_size_t prev_ptt_ms;
    atomic_int ptt_worker_continue;
    pthread_mutex_t mutex;

    ma_engine engine;
};

void set_mic_status(server_context_t* context, mic_status_t status, int playsound, char* sound_dir)
{
    context->mic_status = status;

    pid_t pid = fork();
    if (pid == -1) {
        perror("failed to fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // wpctl set-mute @DEFAULT_AUDIO_SOURCE@ 1
        if (context->mic_status == STATUS_MUTED) {
            printf_debug("Muting\n");
            execlp("wpctl", "wpctl", "set-mute", "@DEFAULT_AUDIO_SOURCE@", "1", NULL);
        } else {
            printf_debug("Unmuting\n");
            execlp("wpctl", "wpctl", "set-mute", "@DEFAULT_AUDIO_SOURCE@", "0", NULL);
        }
    } else {
        if (playsound) {
            char sound_path[PATH_MAX + 12] = { 0 };
            strcpy(sound_path, sound_dir);
            int path_len = strlen(sound_path);
            if (context->mic_status == STATUS_MUTED) {
                strcpy(sound_path + path_len, "/muted.mp3\0");
                printf_debug("Attempting to play sound: \"%s\"\n", sound_path);
                play_sound(&context->engine, sound_path);
            } else {
                strcpy(sound_path + path_len, "/unmuted.mp3\0");
                printf_debug("Attempting to play sound: \"%s\"\n", sound_path);
                play_sound(&context->engine, sound_path);
            }
        }
    }
}

void toggle_mic_status(server_context_t* context, int play_sound, char* sound_dir)
{
    set_mic_status(context, !context->mic_status, play_sound, sound_dir);
}

mic_status_t parse_mic_status()
{
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe failed");
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("failed to fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        close(pipe_fd[0]);

        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            perror("dup2 failed");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd[1]);

        execlp("wpctl", "wpctl", "get-volume", "@DEFAULT_AUDIO_SOURCE@", NULL);
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else {

        close(pipe_fd[1]);

        char buffer[128] = { 0 };
        size_t bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1);
        close(pipe_fd[0]);

        if (bytes_read <= 0) {
            return -1;
        }

        return (strstr(buffer, "[MUTED]") != NULL) ? STATUS_MUTED : STATUS_UNMUTED;
    }
}

typedef struct ptt_args ptt_args_t;
struct ptt_args {
    server_context_t* context;
    int play_sound;
    char* sounds_dir;
};

void* ptt_tracker(void* pthread_args)
{
    ptt_args_t* args = (ptt_args_t*)pthread_args;
    while (args->context->ptt_worker_continue) {
        sleep_ms(50);
        size_t current_time = get_time_ms();

        pthread_mutex_lock(&args->context->mutex);
        if (args->context->mic_status == STATUS_UNMUTED && current_time - args->context->prev_ptt_ms >= 500) {
            set_mic_status(args->context, 0, args->play_sound, args->sounds_dir);
        }
        pthread_mutex_unlock(&args->context->mutex);
    }

    return NULL;
}

int server(int ptt, int play_sound)
{
    char executable_path_buffer[PATH_MAX];
    char* sounds_dir = executable_path(executable_path_buffer);
    int path_len = strlen(sounds_dir);
    const char* sounds = "/sounds\0";
    strcpy(sounds_dir + path_len, sounds);
    printf_debug("SERVER: Sound path: \"%s\"\n", sounds_dir);

    server_context_t context;
    context.mic_status = STATUS_MUTED;
    context.prev_ptt_ms = (size_t)-1;
    context.ptt_worker_continue = 1;

    context.mic_status = parse_mic_status();
    if (ptt && context.mic_status == STATUS_MUTED) {
        set_mic_status(&context, 0, play_sound, sounds_dir);
    }

    int audio_result = init_audio(&context.engine);
    if (audio_result != 0) {
        return audio_result;
    }

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

    pthread_t ptt_thread;
    ptt_args_t ptt_args;
    if (ptt) {
        ptt_args.context = &context;
        ptt_args.play_sound = play_sound;
        ptt_args.sounds_dir = sounds_dir;
        if (pthread_mutex_init(&context.mutex, NULL) != 0) {
            perror("SERVER: failed to initialize ptt mutex");
            exit(EXIT_FAILURE);
        }
        pthread_create(&ptt_thread, NULL, ptt_tracker, &ptt_args);
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
            if (!ptt) {
                toggle_mic_status(&context, play_sound, sounds_dir);
            } else {
                pthread_mutex_lock(&context.mutex);
                // Default it is muted
                // Pressing ptt unmutes
                if (context.mic_status == STATUS_MUTED) {
                    set_mic_status(&context, 1, play_sound, sounds_dir);
                }
                // Repeat presses reset prev_ptt_ms
                context.prev_ptt_ms = get_time_ms();
                pthread_mutex_unlock(&context.mutex);
            }
        }

        if (buf[0] == '0') {
            set_mic_status(&context, STATUS_MUTED, play_sound, sounds_dir);
        }

        if (buf[0] == '1') {
            set_mic_status(&context, STATUS_UNMUTED, play_sound, sounds_dir);
        }

        if (buf[0] == 'q') {
            goto cleanup;
        }
    }

cleanup:
    close(server_fd);
    unlink(SOCKET_PATH);
    if (ptt) {
        context.ptt_worker_continue = 0;
        pthread_join(ptt_thread, NULL);
        pthread_mutex_destroy(&context.mutex);
    }
    deinit_audio(&context.engine);
    return 0;
}
