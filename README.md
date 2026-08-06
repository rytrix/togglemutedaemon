# togglemutedaemon

A simple c program using unix sockets and pipewire to toggle mute. Uses miniaudio to play text to speech notifications.

Designed specifically to work across multiple wayland compositors to attempt to solve my problem with push to talk on wayland.

Also has support for push to talk on gnome, using the p option sending client toggle messages (gnome sends repeat keypresses when holding down a custom keybind).
