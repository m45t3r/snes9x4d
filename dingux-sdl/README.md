# Snes9x4d for Miyoo

Snes9x4d for [MiyooCFW](https://github.com/TriForceX/MiyooCFW) devices.
Should run in both BittBoy and PocketGo, however it is only tested on
PocketGo v1.

## Bulding

Follow those steps to setup Debian 9 environment with compatible toolchain:
<https://github.com/steward-fu/miyoo>. 

Afterwards, run:

    $ make -f Makefile.miyoo

It will generate a binary `snes9x4d` binary. You can them substitute
`sd://emus/snes9x4d` contents with the ones in the generated directory.
