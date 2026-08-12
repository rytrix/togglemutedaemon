# togglemutedaemon

A simple c program using unix sockets and pipewire to toggle mute. Uses miniaudio to play text to speech notifications.

Designed specifically to work across multiple wayland compositors to attempt to solve my problem with push to talk on wayland.

Has support for push to talk on gnome, using the p option sending client toggle messages (gnome sends repeat keypresses when holding down a custom keybind).

Supports global push to talk using /dev/input/eventX interface. (Requires root permissions)

### Usage
s - server/daemon; a(audio), p(push to talk)

c - client; t(toggle mute), 0(unmute), 1(mute) q(exit)

g - global push to talk (requires root); keybind(f12,f11,lalt)
