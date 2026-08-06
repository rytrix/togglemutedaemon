#include "../external/miniaudio.h"
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

#define SOCKET_PATH "/tmp/togglemutedaemon"
#define BUFFER_SIZE 1

#ifdef DEBUG
#define printf_debug printf
#else
#define printf_debug //
#endif

int muted = 0;
atomic_size_t prev_ptt_ms = 0;
atomic_int ptt_worker_continue = 1;
pthread_mutex_t toggle_mute_lock;

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

void set_mute(int toggle, int playsound, char* sound_dir)
{
    muted = toggle;

    pid_t pid = fork();
    if (pid == -1) {
        perror("failed to fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // wpctl set-mute @DEFAULT_AUDIO_SOURCE@ 1
        if (muted == 1) {
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
            if (muted == 1) {
                strcpy(sound_path + path_len, "/muted.mp3\0");
                printf_debug("Attempting to play sound: \"%s\"\n", sound_path);
                play_sound(sound_path);
            } else {
                strcpy(sound_path + path_len, "/unmuted.mp3\0");
                printf_debug("Attempting to play sound: \"%s\"\n", sound_path);
                play_sound(sound_path);
            }
        }
    }
}

void toggle_mute(int play_sound, char* sound_dir)
{
    set_mute(!muted, play_sound, sound_dir);
}

int parse_mute()
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

        return (strstr(buffer, "[MUTED]") != NULL) ? 0 : 1;
    }
}

size_t get_time_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (size_t)(ts.tv_sec * 1000) + (size_t)(ts.tv_nsec / 1000000);
}

void sleep_ms(size_t milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

struct PttArgs {
    int play_sound;
    char* sounds_dir;
};
typedef struct PttArgs PttArgs_t;

void* ptt_tracker(void* pthread_args)
{
    PttArgs_t* args = (PttArgs_t*)pthread_args;
    while (ptt_worker_continue) {
        sleep_ms(50);
        size_t current_time = get_time_ms();

        pthread_mutex_lock(&toggle_mute_lock);
        if (muted == 0 && current_time - prev_ptt_ms >= 500) {
            set_mute(1, args->play_sound, args->sounds_dir);
        }
        pthread_mutex_unlock(&toggle_mute_lock);
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

    muted = parse_mute();
    if (ptt && muted) {
        set_mute(1, play_sound, sounds_dir);
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
    PttArgs_t ptt_args;
    if (ptt) {
        ptt_args.play_sound = play_sound;
        ptt_args.sounds_dir = sounds_dir;
        if (pthread_mutex_init(&toggle_mute_lock, NULL) != 0) {
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
                toggle_mute(play_sound, sounds_dir);
            } else {
                pthread_mutex_lock(&toggle_mute_lock);
                // Default it is muted
                // Pressing ptt unmutes
                if (muted == 1) {
                    set_mute(0, play_sound, sounds_dir);
                }
                // Repeat presses reset prev_ptt_ms
                prev_ptt_ms = get_time_ms();
                pthread_mutex_unlock(&toggle_mute_lock);
            }
        }

        if (buf[0] == '0') {
            set_mute(0, play_sound, sounds_dir);
        }

        if (buf[0] == '1') {
            set_mute(1, play_sound, sounds_dir);
        }

        if (buf[0] == 'q') {
            goto cleanup;
        }
    }

cleanup:
    close(server_fd);
    unlink(SOCKET_PATH);
    if (ptt) {
        ptt_worker_continue = 0;
        pthread_join(ptt_thread, NULL);
    }
    pthread_mutex_destroy(&toggle_mute_lock);
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
    const char* message;
};
typedef struct Args Args_t;

Args_t args_default()
{
    Args_t args;
    args.server = ARG_NOT_PRESENT;
    args.client = ARG_NOT_PRESENT;
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

int main(int argc, char** argv)
{
    Args_t args = parse_args(argc, argv);
    if (args.server == ARG_PRESENT) {
        int audio = 0;
        int ptt = 0;
        if (args.audio == ARG_PRESENT) {
            audio = 1;
            int audio_result = init_audio();
            if (audio_result != 0) {
                return audio_result;
            }
        }
        if (args.push_to_talk) {
            ptt = 1;
        }
        server(ptt, audio);
        if (audio) {
            deinit_audio();
        }
    } else if (args.client == ARG_PRESENT) {
        if (args.message == NULL) {
            usage(argv[0]);
        }
        client(*args.message);
    } else {
        usage(argv[0]);
    }

    return 0;
}
