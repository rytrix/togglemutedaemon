#include "string_to_key.h"

#include <stdio.h>
#include <linux/input-event-codes.h>
#include <string.h>

#include "debug.h"

typedef struct key_string key_string_t;
struct key_string {
    char string[32];
    int key;
};

static key_string_t key_strings[] = {
    { .string = "1", .key = KEY_1 },
    { .string = "2", .key = KEY_2 },
    { .string = "3", .key = KEY_3 },
    { .string = "4", .key = KEY_4 },
    { .string = "5", .key = KEY_5 },
    { .string = "6", .key = KEY_6 },
    { .string = "7", .key = KEY_7 },
    { .string = "8", .key = KEY_8 },
    { .string = "9", .key = KEY_9 },
    { .string = "0", .key = KEY_0 },
    { .string = "q", .key = KEY_Q },
    { .string = "w", .key = KEY_W },
    { .string = "e", .key = KEY_E },
    { .string = "r", .key = KEY_R },
    { .string = "t", .key = KEY_T },
    { .string = "y", .key = KEY_Y },
    { .string = "u", .key = KEY_U },
    { .string = "i", .key = KEY_I },
    { .string = "o", .key = KEY_O },
    { .string = "p", .key = KEY_P },
    { .string = "a", .key = KEY_A },
    { .string = "s", .key = KEY_S },
    { .string = "d", .key = KEY_D },
    { .string = "f", .key = KEY_F },
    { .string = "g", .key = KEY_G },
    { .string = "h", .key = KEY_H },
    { .string = "j", .key = KEY_J },
    { .string = "k", .key = KEY_K },
    { .string = "l", .key = KEY_L },
    { .string = "z", .key = KEY_Z },
    { .string = "x", .key = KEY_X },
    { .string = "c", .key = KEY_C },
    { .string = "v", .key = KEY_V },
    { .string = "b", .key = KEY_B },
    { .string = "n", .key = KEY_N },
    { .string = "m", .key = KEY_M },
    { .string = "f1", .key = KEY_F1 },
    { .string = "f2", .key = KEY_F2 },
    { .string = "f3", .key = KEY_F3 },
    { .string = "f4", .key = KEY_F4 },
    { .string = "f5", .key = KEY_F5 },
    { .string = "f6", .key = KEY_F6 },
    { .string = "f7", .key = KEY_F7 },
    { .string = "f8", .key = KEY_F8 },
    { .string = "f9", .key = KEY_F9 },
    { .string = "f10", .key = KEY_F10 },
    { .string = "f11", .key = KEY_F11 },
    { .string = "f12", .key = KEY_F12 },
    { .string = "f13", .key = KEY_F13 },
    { .string = "f14", .key = KEY_F14 },
    { .string = "f15", .key = KEY_F15 },
    { .string = "f16", .key = KEY_F16 },
    { .string = "f17", .key = KEY_F17 },
    { .string = "f18", .key = KEY_F18 },
    { .string = "f19", .key = KEY_F19 },
    { .string = "f20", .key = KEY_F20 },
    { .string = "f21", .key = KEY_F21 },
    { .string = "f22", .key = KEY_F22 },
    { .string = "f23", .key = KEY_F23 },
    { .string = "f24", .key = KEY_F24 },
    { .string = "esc", .key = KEY_ESC },
    { .string = "minus", .key = KEY_MINUS },
    { .string = "equal", .key = KEY_EQUAL },
    { .string = "backspace", .key = KEY_BACKSPACE },
    { .string = "tab", .key = KEY_TAB },
    { .string = "lbrace", .key = KEY_LEFTBRACE },
    { .string = "rbrace", .key = KEY_RIGHTBRACE },
    { .string = "enter", .key = KEY_ENTER },
    { .string = "lctrl", .key = KEY_LEFTCTRL },
    { .string = "semicolon", .key = KEY_SEMICOLON },
    { .string = "apostrophe", .key = KEY_APOSTROPHE },
    { .string = "lshift", .key = KEY_LEFTSHIFT },
    { .string = "backslash", .key = KEY_BACKSLASH },
    { .string = "comma", .key = KEY_COMMA },
    { .string = "dot", .key = KEY_DOT },
    { .string = "slash", .key = KEY_SLASH },
    { .string = "lalt", .key = KEY_LEFTALT },
    { .string = "space", .key = KEY_SPACE },
    { .string = "capslock", .key = KEY_CAPSLOCK },
    { .string = "numlock", .key = KEY_NUMLOCK },
    { .string = "scrolllock", .key = KEY_SCROLLLOCK },
    { .string = "rctrl", .key = KEY_RIGHTCTRL },
    { .string = "ralt", .key = KEY_RIGHTALT },
    { .string = "home", .key = KEY_HOME },
    { .string = "up", .key = KEY_UP },
    { .string = "pageup", .key = KEY_PAGEUP },
    { .string = "left", .key = KEY_LEFT },
    { .string = "right", .key = KEY_RIGHT },
    { .string = "end", .key = KEY_END },
    { .string = "down", .key = KEY_DOWN },
    { .string = "pagedown", .key = KEY_PAGEDOWN },
    { .string = "insert", .key = KEY_INSERT },
    { .string = "delete", .key = KEY_DELETE },
};

int string_to_key(const char* string)
{
    for (int i = 0; i < sizeof(key_strings)/sizeof(key_string_t); i++) {
        if (strcmp(string, key_strings[i].string) == 0) {
            printf_debug("Found key %s\n", string);
            return key_strings[i].key;
        }
    }

    return -1;
}
