# togglemutedaemon

A simple c program using unix sockets and pipewire to toggle mute. Uses miniaudio to play text to speech notifications.

Designed specifically to work across multiple wayland compositors to attempt to solve my problem with push to talk on wayland.

Has support for push to talk on gnome, using the p option sending client toggle messages (gnome sends repeat keypresses when holding down a custom keybind).

Supports global push to talk using /dev/input/eventX interface. (Requires root permissions)

### Usage
s - server/daemon; a(audio), p(push to talk)

c - client; t(toggle mute), 0(unmute), 1(mute) q(exit)

g - global push to talk (requires root); keybind(f12,f11,lalt)

### Guide
Run a server using the following command
```bash
./out/togglemutedaemon s
```
Then send client messages to change pipewire's mute/unmuted state
```bash
./out/togglemutedaemon c t
```
Client toggle could be bound to a compositor keybind, or the global push to talk option can be used
```bash
sudo ./out/togglemutedaemon g f12
```
This option will allow you to select a keyboard device that the push to talk key is located on and send client messages when the key is pressed and released

### Building
There are no notable dependencies outside of gcc and being on a linux system, miniaudio is vendored
```bash
make
```
The install option is also present and will install to "$HOME/.local/bin/"
```bash
make install
```
The program can then also be uninstalled using the following
```bash
make uninstall
```
