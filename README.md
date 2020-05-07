# Snes9x4d for Miyoo

Snes9x4d for [MiyooCFW](https://github.com/TriForceX/MiyooCFW) devices.
Should run in both BittBoy and PocketGo, however it is only tested on
PocketGo v1.

## Bulding

Follow those steps to setup Debian 9 environment with compatible toolchain:
<https://github.com/steward-fu/miyoo>.

Afterwards, run:

    $ make

It will generate a binary `snes9x4d` binary. You can them substitute
`sd://emus/snes9x4d` contents with the ones in the generated directory.

You can build this binary using [PGO](https://en.wikipedia.org/wiki/Profile-guided_optimization).
To do this, first:

    $ make PGO=GENERATE

To generate a binary with instrumentation. Put this in your Miyoo and play a
little, but keep in mind that a PGO binary is very slow (so be patient, it
is worth it). Afterwards, copy all files in `sd://emus/snes9x4d` to this
directory and run:

    $ make PGO=APPLY

To apply optimizations. A `profile.zip` file is included with a small
playthrough during the first level of `Super Mario World`, but may be not
updated. Also, keep in mind that if you want the best performance in your
specific game it may be better to generate your playthrough of it.
