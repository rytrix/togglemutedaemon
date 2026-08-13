#include "watch_key.h"

#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "client.h"
#include "debug.h"
#include "string_to_key.h"

struct device_result {
    int fd;
    char file_path[256];
    char name[256];
};
typedef struct device_result device_result_t;

int check_device_for_key(device_result_t* result, int key)
{
#define IS_BIT_SET(bit, array) ((array[(bit) / 8] & (1 << ((bit) % 8))) != 0)

    result->fd = open(result->file_path, O_RDONLY);
    if (result->fd < 0) {
        return -1;
    }

    uint8_t ev_bitmask[(EV_MAX / 8) + 1] = { 0 };
    if (ioctl(result->fd, EVIOCGBIT(0, sizeof(ev_bitmask)), ev_bitmask) < 0) {
        close(result->fd);
        result->fd = -1;
        return -1;
    }

    if (!IS_BIT_SET(EV_KEY, ev_bitmask)) {
        close(result->fd);
        result->fd = -1;
        return -1;
    }

    uint8_t key_bitmask[(KEY_MAX / 8) + 1] = { 0 };
    if (ioctl(result->fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask) < 0) {
        close(result->fd);
        result->fd = -1;
        return -1;
    }

    if (IS_BIT_SET(key, key_bitmask)) {
        const char* default_name = "Unknown Device";
        memcpy(result->name, default_name, strlen(default_name));
        ioctl(result->fd, EVIOCGNAME(sizeof(result->name)), result->name);

        printf("Found Suitable Device: %s\nName: %s\n", result->file_path, result->name);
    } else {
        close(result->fd);
        result->fd = -1;
        return -1;
    }

    return 0;

#undef IS_BIT_SET
}

int get_user_selection(int max_options) {
    char buffer[64];
    int selection = -1;

    while (1) {
        printf("Enter selection (0-%d): ", max_options - 1);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return -1;
        }

        if (sscanf(buffer, "%d", &selection) == 1) {
            if (selection >= 0 && selection < max_options) {
                return selection;
            }
        }

        return -1;
    }
}

void watch_key_internal(int fd, int key)
{
    struct input_event ev;

    while (1) {
        ssize_t bytes = read(fd, &ev, sizeof(ev));
        if (bytes < (ssize_t)sizeof(ev)) {
            perror("Error reading event");
            break;
        }

        if (ev.type == EV_KEY) {
            // printf("Detected key %d\n", ev.code);
            if (ev.code == key) {
                if (ev.value == 1) {
                    // Keydown
                    client('1');
                }
                if (ev.value == 0) {
                    // Released
                    client('0');
                }
            }
        }
    }
}

void watch_key(const char* key)
{
    const char* device_dir = "/dev/input";
    DIR* dir = opendir(device_dir);

    device_result_t results[32] = { 0 };
    int results_size = 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "event", 5) == 0) {
            snprintf(results[results_size].file_path, sizeof(results[results_size].file_path), "%s/%s", device_dir, entry->d_name);

            printf_debug("%s \n", results[results_size].file_path);
            int result = check_device_for_key(&results[results_size], KEY_F12);
            if (result >= 0) {
                results_size++;
            }
        }
    }

    for (int i = 0; i < results_size; i++) {
        printf("%d: %s\n", i, results[i].name);
    }
    int selection = get_user_selection(results_size);
    if (selection < 0) {
        printf("Invalid selection\n");
        return;
    }

    int key_value = -1;
    if (key != NULL) {
        key_value = string_to_key(key);
    }

    if (key_value == -1) {
        key_value = KEY_F12;
    }

    watch_key_internal(results[selection].fd, key_value);
}
