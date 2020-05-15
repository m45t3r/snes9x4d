# Snes9x4d for Miyoo

Snes9x4d for [MiyooCFW](https://github.com/TriForceX/MiyooCFW) devices.
Should run in both BittBoy and PocketGo, however it is only tested on
PocketGo v1.

## Usage

Assuming that you're using the latest version of
[MiyooCFW](https://github.com/TriForceX/MiyooCFW), go to the
[releases](https://github.com/m45t3r/snes9x4d-miyoo/releases/) tab and
download the latest version of `snes9x4d` file. Put this file in
`sd://emus/snes9x4d`, substituing the file that is in there. That's it!

To check if you're running the latest version, open the emulator and press
`R` button to open menu, and check the title. It should be written something
like `Snes9x4d vYYYY.MM.DD for Miyoo` (where `YYYY.MM.DD` is a date).

**Optional:** You can overclock your device to either `732MHz` or `798MHz` to
get better performance. To do this press `Select` when `SNES` is selected in
`Emulators` menu, select `EDIT SNES` and adjust `CPU Clock` to either `702`
(default), `732` or `798`. Not all devices support all overclock options, so
if your device freeze reduce the overclock or return to default.

## Options

This emulator has multiple options that can be accessed using `Reset` button in
BittBoy/PocketGo. Some of them may improve performance at the expense of
quality. Some of them are described below:

- Transparency (default `On`): enables/disables transparency effects in games.
  Disabling it may improve performance, but it may also break some effects in
  games and even make some of them unplayable
- Full Screen (default `False`): enables/disables full screen. There is a small
  performance impact for it, and it also makes the aspect ratio wrong
- Billinear Filtering (default `False`): enables/disables bilinear filtering.
  Only makes sense when `Full Screen` is also `True`. This makes the image in
  full screen smoother at the expense of performance
- Frameskip (default `Auto`): sets frameskipping. It works by skipping some
  frames, for example assuming a NTSC (USA or Japanese) ROM with 60Hz of
  refresh rate, setting Frameskip to `60/2` will skip every other frame
  (rendering only 30 frames per second instead of 60). `Auto` tries to skip
  frames as much as necessary to keep full speed, but in some heavy games
  (like `Star Fox`) it is better to set a fixed frameskip to make the game
  stutter less
- Sound Rate (default `32000`): sets sound rate. The official sound rate of
  SNES is ~32000 so this is the recommended value to have the best available
  sound quality, but you may reduce it to improve performance
- Stereo (default `False`): enables/disables stereo mixing (making a Mono
  output). Disabling it improves performance, but the Mono mixing results in
  some missing instruments in games

## Setup environment

First, you need to setup a toolchain. This code is tested with:

- https://github.com/steward-fu/miyoo
- https://github.com/bittboy/buildroot/

The first one is easier to setup since it has some instructions and also the
toolchain is pre-built, but it is also based on an older version of GCC (7.3.0)
using [`uclibc`](https://uclibc.org/).

The second one needs to be compiled from source, but is based on a much newer
version of GCC (9.3.0) and also uses [`musl`](https://musl.libc.org/), a newer
implementation of C stdlib.

Assuming that you're going to use <https://github.com/bittboy/buildroot/>, the
first step is to have a Linux environment setup. I used a
[Debian 9](https://www.debian.org/releases/stretch/debian-installer/) VM for
this.

The only necessary package outside the minimal setup is `build-essential` and
`git`, so install it first:

    $ sudo apt install build-essential git

Afterwards, clone `https://github.com/bittboy/buildroot/`, and run:

    $ cd buildroot
    $ make

This will take a while and may need some retries (for example, I had to run
`make` three times to finish the build). After building the toolchain, you
should have a `output` inside `buildroot` directory.

You need to add the cross-compilers to your `PATH`. So run:

    # In buildroot directory
    $ export PATH=$PWD/output/host/bin:$PATH

## Bulding

After setting up your environment, run:

    $ make

It will generate a binary `snes9x4d` binary. You can them substitute
`sd://emus/snes9x4d` contents with the ones in the generated directory.

You can build this binary using
[PGO](https://en.wikipedia.org/wiki/Profile-guided_optimization).
To do this, first:

    $ make PGO=GENERATE

To generate a binary with instrumentation. Put this in your Miyoo and play a
little, but keep in mind that a PGO binary is very slow (so be patient, it
is worth it). Afterwards, copy  `profile` directory from `sd://emus/snes9x4d`
to the root of the project and run:

    $ make PGO=APPLY

To apply optimizations. A `profile.zip` file is included with each release with
a playthrough of some game, but it may not be updated with `master` branch.
Also, keep in mind that if you want the best performance in your
specific game it may be better to generate your playthrough of it.

## Porting

While this fork removes support for any other device outside Miyoo (since I
don't have the infrastructure to maintain the ports), it should be
straightforward to port this code for a new device, specially for a Dingux-like
(Linux+SDL) devices.

Each specific device feature is behind compiler-flags so it shouldn't affect
compilation for other targets. Also this port has many code clean-ups, so it
should be easier to navigate compared to other `snes9x4d` ports.

Some tips to port this project to another Dingoo-like device:

- You can start by getting your toolchain working and creating a new `Makefile`
  for your device (just make a copy of the original one and change some things)
- Enable/disable some compile toggles (see `CCFLAGS` in `Makefile`) according
  to your device. Some of the compile toggles are described below:
    + `CPU_SHUTDOWN`, `VAR_CYCLES`, `SPC700_SHUTDOWN` are speed hacks available
      from `snes9x` itself, so they should be safe to enable
    + Unless your device is an ARM device, remove `__ARM__` feature flag since
      will enable ARM specific optimizations (assembly)
    + `VIDEO_MODE` defines some configuration related to video. For now there
      are 3 pre-defined video modes: `1` for `320x240`, `2` for `400x240` and
      `3` for `480x272`. If your device has another resolution, you will need
      to define a new video mode (check `src/sdlmenu/sdlvideo.h` file) and a
      new scaler function (see `src/sdlmenu/scaler.cpp`)
    + `BILINEAR_SCALE` is an alternative scale method that has higher quality
      than the default one (it is slightly more expansive too). For now, it is
      only available for `VIDEO_MODE=1`, so if your device uses any other
      video mode it is better to disable it
    + Also it is probably a good idea to disable `SPC700_ASM` (since the code
      only has an ARM version of it). You can always enable it later
    + `_ZAURUS` is a compile toggle that is used in many parts of the project,
      disabling many features that doesn't make sense to embedded devices. So
      it should be enabled (and there are good chances that the code will
      simply not build without it)
    + `TL_COLOR_OPS` is an interesting optimization from
      [drowsnug95](https://github.com/drowsnug95/snes9x4d-rs90/) port. It
      allows color operations without a lookup table, and it is significantly
      faster. But it is also somewhat wrong, so it breaks colors in some games,
      like the menu in F-Zero and the intro in Street Fighter Alpha 2 (but not
      in-game). If your device is slow it is worth it, but if your device is
      fast enough to run games at full speed without it is better to disable it
- If your device is like Miyoo and doesn't have an standard Dingoo layout, you
  should define your own custom layout by creating a file
  `src/sdlmenu/<device>.h` and mapping the SDL buttons accordingly (see
  `src/sdlmenu/miyoo.h` for an example). Afterwards, just import your custom
  mapping conditionally in `src/sdlmenu/sdlmain.cpp` and it should work
