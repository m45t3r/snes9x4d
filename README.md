# Snes9x4d for Miyoo

Snes9x4d for [MiyooCFW](https://github.com/TriForceX/MiyooCFW/) devices.
Should run in both BittBoy and PocketGo, however it is only tested on
PocketGo v1.

## Usage

Assuming that you're using the latest version of
[MiyooCFW](https://github.com/TriForceX/MiyooCFW/), go to the
[releases](https://github.com/m45t3r/snes9x4d-miyoo/releases/) tab and
download the latest version of `snes9x4d` file. Put this file in
`sd://emus/snes9x4d`, substituting the file that is in there. That's it!

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
BittBoy/PocketGo. They may improve performance at the expense of quality. Some
of them are described below:

- Transparency (default `True`): enables/disables transparency effects in
  games. Disabling it may improve performance, but it may also break some
  effects in games and even make some of them unplayable because of missing
  effects
- Full Screen (default `False`): enables/disables full screen. There is a small
  performance impact for it, and it also makes the aspect ratio wrong
- Billinear Filtering (default `False`): enables/disables bilinear filtering.
  Only makes sense when `Full Screen` is also `True`. This makes the image in
  full screen smoother at the expense of performance
- Frameskip (default `Auto`): sets frameskipping. It works by skipping some
  frames, for example assuming a NTSC (USA or Japanese) ROM with 60Hz of
  refresh rate, setting Frameskip to `60/2` will skip every other frame
  (rendering only 30 frames per second instead of 60). `Auto` tries to skip
  frames as much as necessary to keep full speed
- Auto Frameskip (default `Max. 3`): defines the maximum frames to skip when
  `Frameskip` is set to `Auto`. You can increase it to allow the game to render
  faster, at the expense of more stuttering and input lag. My recommendation
  is to not use a value bigger than `Max. 4`. For games that need fast reaction
  times you may want to decrease this value
- Sound Rate (default `32000`): sets sound rate. The official sound rate of
  SNES is ~32000 so this is the minimum recommended value to have a good
  sound quality, but you may reduce it to improve performance. You can also
  disable sound by setting this option to `off`
- Stereo (default `False`): enables/disables stereo mixing. Disabling it
  improves performance, but the Mono mixing results in some missing instruments
  in games

My personal performance tip is to disable Stereo and Transparency. This makes
the most performance gains in games where those options doesn't break anything.

Setting Frameskip to `Auto` is the only current way to have a framerate limit,
so it is recommended to always use it. You can adjust the maximum number of
frames to skip using `Auto Frameskip` option. Still, `Auto Frameskip` does
bring some overhead so it may make sense to use fixed Frameskip values
sometimes, but keep it in mind that the game can go too fast in areas where
there is little going on (for example, game menus).

If you want to play with Sound Rate, `22050` still gives a very good sound, but
in my experience it doesn't improve performance that much. `16000` is
acceptable and anything below it is better to simply disable sound (that gives
much better performance gains).

## Setup environment

First, you need to setup a toolchain. This code is tested with:

- https://github.com/steward-fu/miyoo/
- https://github.com/bittboy/buildroot/

The first one is easier to setup since it has some instructions and also the
toolchain is pre-built, but it is also based on an older version of GCC (7.3.0)
using [`uclibc`](https://uclibc.org/).

The second one needs to be compiled from source, but is based on a much newer
version of GCC (10.1.0) and also uses [`musl`](https://musl.libc.org/), a newer
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
compilation for other targets. This port has many code clean-ups, so it should
be easier to navigate compared to other `snes9x4d` ports. Also this port brings
some additional optimizations and features not available in other `snes9x4d`
ports, like the color operations code from `Snes9x 1.60`, bilinear(ish)
filtering from `PocketSNES`, `LAGFIX` from `snes9x2002`, etc.

Some tips to port this project to another Dingoo-like device:

- You can start by getting your toolchain working and creating a new `Makefile`
  for your device (just make a copy of the original one and change some things)
- Enable/disable some compile toggles (see `CCFLAGS` in `Makefile`) according
  to your device. Some of the compile toggles are described below:
    + `SPC700_SHUTDOWN` is a speed hack available from `Snes9x` itself, so it
      should be safe to enable.
    + `FAST_LSB_WORD_ACCESS` seems to speed-up the code access to some memory
      operations. Recommended if your target supports it
    + `FAST_ALIGNED_LSB_WORD_ACCESS` is a similar to above, but only optimizes
      aligned memory access. If your target doesn't support the above, this one
      may work
    + `VIDEO_MODE` defines some configuration related to video. For now there
      are 3 pre-defined video modes: `1` for `320x240`, `2` for `400x240` and
      `3` for `480x272`. If your device has another resolution, you will need
      to define a new video mode (check `src/sdlmenu/sdlvideo.h` file) and a
      new scaler function (see `src/sdlmenu/scaler.cpp`)
    + `BILINEAR_SCALE` is an alternative scale method that has higher quality
      than the default one (it is slightly more expansive too). For now, it is
      only available for `VIDEO_MODE=1`, so if your device uses any other
      video mode it is better to disable it
    + `MIYOO` is obviously used only for Miyoo devices, so you shouldn't define
      it for your own device. But looking where it is used in the code should
      help you to define your own device-specific code
    + `C4_OLD` (this is a Makefile flag) use an older version of C(x)4 chip
      emulation that uses approximations. Probably faster but more inaccurate.
      Cx4 chip was only used in 2 games: Mega Man X 2 and 3
    + `LAGFIX` this flag reduces the input lag in one frame (~16ms). Backported
      from `snes9x2002`, and you can read more about it here:
      https://www.libretro.com/index.php/core-progress-snes9x-2002-input-lag-reduced-by-1-2-frames/
    + `SNESADVANCE_SPEEDHACKS` enables some speedhacks that can be loaded by a
      special crafted `snesadvanced.dat` file (there is an example in this
      repository). Just put this file in `~/.snes9x4d-ng/snesadvanced.dat` and
      load a ROM, if there is an available speedhack it will be automatically
      applied. My own testing is inconclusive if this brings any benefit in
      speed, though
- If your device is like Miyoo and doesn't have an standard Dingoo layout, you
  should define your own custom layout by creating a file
  `src/sdlmenu/<device>.h` and mapping the SDL buttons accordingly (see
  `src/sdlmenu/miyoo.h` for an example). Afterwards, just import your custom
  mapping conditionally in `src/sdlmenu/sdlmain.cpp` and it should work
