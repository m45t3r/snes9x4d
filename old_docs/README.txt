Snes9x4P
========
(c)^SiENcE^2010.10.10
(c)skeezix, 2010
(c)Ivanovic, 2010-2011


Credit to SiENcE
(http://boards.dingoonity.org/dingux-releases/snes9x4d-%28for-dingux%29-v20100330/)
for doing most of the work. Besides this credits go to skeezix for doing the
initial port to the Pandora. Thanks to pickle for creating his fantastic
picklelauncher which is included in this package, too. Besides this emulator is
using a modified libSDL from notaz which e.g. allows hardware accelerated
scaling.

This SNES emulator is running pretty well; for now there are no ASM CPU cores in
place, so try overclocking to keep speed up where needed. This will be remedied
once someone finds the time and energy to work on things (patches are welcome!
;) ). This emulator should run all games at full speed pretty easily if you
overclock a little. Press SPACE to bring up the menu. The menu lets you change
scaling mode, display FPS, quit, ...


========
Controls
========
Normal "gamepad" buttons are mapped to the same places where the SNES controller
has them. Below are additional buttons that are used to control the emulator
itself. All capital letters mean "special" keyboard keys, a single, capital
letter means a gamepad button and small letters mean a button on the Pandoras
keyboard. If you have to press several buttons at the same time this is marked
using a '+' between the keys.

Keys while inside the emulator and not in the emulators menu:
SPACE - opens the menu
ESC or Start+Select+Y - exit the emulator
Start+Select+X - reset the currently running rom
Start+R - create a savestate in the currently selected slot
Start+L - load the savestate from the currently selected slot
s - change the current display mode
t - toggle turbo mode (on or off; turbo mode setting is not stored!)

Keys inside the (in emulator) menu:
X - abort (and go back to the game)
B - select current entry (execute something)
Up or Down - change the selected entry
Left or Right - change the value for the selected entry

Keys inside picklelauncher (the touchscreen works, too!):
Esc - leave picklelauncher
Up - one entry up in the list
Down - one entry down in the list
Right - ten entries down in the list
Left - ten entries up in the list
A - switch into the marked folder
X - one folder down
B - switch into the marked folder or start the selected rom
any letter on the keyboard - filter for roms with the selected letter(s)
backspace - remove last entered letter


==================
Compiling Snes9x4P
==================
Feel free to get yourself a checkout of it yourself and improve the emulator.
The sources are now available using github to ease collaboration.
* Clone the git repository with the latest sources
    git clone git://github.com/ivanovic-wesnoth/Snes9x4P.git

* Change into the checkout dir:
    cd Snes9x4P

* Edit Makefile.pandora and put in the correct path in the var TOOLCHAINDIR.

* build Snes9x4P:
    make -f Makefile.pandora
  (or, to directly create a pnd)

* Or if you want to directly create a pnd you can omit the last step and call
    make -f Makefile.pandora pnd


======================
Changelog for Snes9X4P
======================
This is meant to be a list of the most important changes over the last releases
of the emulator Snes9x4P. The listed versions numbers are identical to the
version numbers reflected by the PXML.
It is split into three parts:
1) At the top are the most recent changes that are specifc to the pandora port
of Snes9x4X that were mostly done by Ivanovic.
2) Below are some older changes that are specific to the pandora port. Most of
those changes were done by Ivanovic unless mentioned differently. What is not
mentioned are the things that skeezix did to port this emulator to the pandora
since the details are unknown.
3) At the buttom are the changes that were done in Snes9x4X compared to mainline
Snes9x and were done by SiENcE to allow porting to various decives.

Changes in "1.39ff.20120226.1" compared to "1.39ff.20111215.3":
* Finally fixed OS Version detection.
* Changed default scaling mode to "2x2 no-AA" to make sure that the emulator always starts even if problems with hardware acceleration might occur (eg. with the experimental kernel or further problems due to libSDL and the OS version).

Changes in "1.39ff.20111215.3" compared to "1.39ff.20111215.2":
* Switched to the latest libSDL from notaz as of his thread, not the same as in the firmware. Also changed to script invoking it so that the lib is actually preloaded correctly.

Changes in "1.39ff.20111215.2" compared to "1.39ff.20111215.1":
* Compiled using a more recent toolchain (including additional compiler flags).
* Detect the OS version and use the system libsdl if >=HF7 is used.
* Ship the latest libsdl from Notaz as of SuperZaxxon Beta 1.1.

Changes in "1.39ff.20111215.1" compared to "1.39ff.20111213.1":
* Added a "toggle turbo mode" key and mapped it to 't' on the keyboard.
* Added a battery level indicator in the menu.
* Increased the number of savestate slots to 10 (instead of 4).
* Updated preview pics.

Changes in "1.39ff.20111213.1" compared to "1.39ff.20111208.1":
* Don't quit the emulator when pressing the 'q' button on the Keyboard. Quiting
  now works using 'Esc' (fn+q) or via the menu.
* Completely reworked the menu (feedback welcome!).
* Added an option to toggle vsync to the menu. Beware that turning off vsync will
  remove the frame limiter and many games will run significantly too fast.
* Some rendering code cleanups leading to tiny (negligible!) speed improvements
  for the HW scaled modes as well as HiRes games when using 2x2 no-AA and the rom
  requires a width of 512px.
* Update of the README to list credits and point to the new repo at github.

Changes in "1.39ff.20111208.1" compared to "1.39ff.20111207.1":
* Added the possibility to cut black borders in the GUI (useful for SF2- and
  Final Fight-series and Batman Returns)
* Values for cut borders automatically get stored and restored per game

Changes in "1.39ff.20111207.1" compared to "1.39ff.20111205.2":
* Make sure the (rather useless!) fps counter is displayed on the game screen
  and not in some "off screen corner"
* Reenable display of messages like "State saved" and allign them nicely in the
  line above the fps counter.
* Allow to exit the emulator with 'Esc' additonally to exiting via the menu or
  the key 'q'.
* Updated the readme file to include the Pandora specific changelog and info
  about compilation as well as other things.

Changes in "1.39ff.20111205.2" compared to "1.39ff.20111205.1":
* New version of picklelauncher (based on r8 of pickles svn repo).

Changes in "1.39ff.20111205.1" compared to "1.39ff.20111204.1":
* Allow hardware scaling for HiRes Games, too.
* Default to the hardware scaled 4:3 mode when nothing is/was selected yet or an
  invalid mode is selected.
* Only display scaling modes that are actually supported (limited support of
  scaling modes for HiRes (no scale2x and smooth)).
* Fixed display of the framecounter when in HW scaling mode.
* Fixed switching through scaling modes using the keyboard key 's' to include
  all modes.
* VSync should now work correctly again.

Changes in "1.39ff.20111204.1" compared to "1.39ff.20111010.2":
* Added notaz libsdl and based on this added some hardware scaling capabilities
  (all using the full height but only work for LowRes Roms (no Secret of Mana)):
  - fullscreen
  - 4:3
  - 8:7 (aspect ratio used internally for NTSC)
  - 8:7,5 (aspect ratio used internally for PAL)
* Fixed menu to display more nicely when switching through display modes.

Changes in "1.39ff.20111010.2" compared to "1.39ff.20111010.1"
* Safe rom specific settings like the savestate slot used, display mode, ...
  into ROMNAME.scfg in the folder where the SRAMs and savestates are stored,
  too.

Changes in "1.39ff.20111010.1" compared to "1.39ff.20110310.2"
* Allow to switch to alternative sound decoding mode using the emulator menu.
  This is required to make sounds effects sound better for some games, especially
  games from squaresoft.
* Fixed the run script to actually allow the usage of an "args.txt" file to
  supply different commandline arguments to the emulator.


Older, pre repo.o.o changes not following the latest version numbering scheme:

Changes in v20110310:
* Updated to the latest version of PickleLauncher.

Changes in v20110214:
* Support subfolders in 7z archives. Please be aware that it will take *ages* to
  load if you select a rom from a larger 7z archive since the whole archive is
  processed during temporary unpacking!

Changes in v20110213:
* Fixed 7z support. Now it should work "for real" even with larger archives with
  several files and if there are things like spaces and more than one '.' in the
  filename.
* Updated to the latest snes9x4X version. This introduces eg support for
  switching of transparency which can give some more speed but should not be
  required on the pandora (thus transparency is active by default).
* Used the latest toolchain to compile it. This might lead to some really tiny
  speed improvements, though I would not bet on it...

Changes in v20101111:
* Added Scale2x support for LowRes games. This is basically the "normal" mode
  with some slight smoothing, speed is close to "2x2 no-AA" mode. Thanks to Pickle
  for implementing this.
* Fixed display of HiRes games in 2x2 mode, now *real* HiRes is used there
  (meaning that fonts should look decent now!).

Changes in v20101106:
* Added smooth scaling support for LowRes games. This currently requires massive
  overclocking (>=800MHz) to run rather smooth. Thanks to WizardStan for
  implementing this.

Changes in v20101028:
* All games, no matter if LowRes or HiRes, PAL or NTSC should now be centered
  horizontally as well as vertically.
* Tiny (really tiny!) performance improvements.

Changes in v20101027:
* Center the screen when playing a HiRes game (eg Secret of Mana).
* Implement 3x2 display mode for HiRes games.


Generic Snes9x4X changes:

ver 20101010
------------
-ROM-BROWSER added
 *only if no ROM is specified during startup via: ./Snes9x4D.gpe romfile.smc
-CHEAT (*.cht files) support added
 *put a *.cht files for your ROMs into the SRAM Folder ".snes96_snapshots"
-Bugfixes and Menu fixes
-Soundbug when quitting removed
-NETPLAY Client support added (not playable yet - only for testing)
 *someone have to host a Snes9x session (Snes9x-1.52_fix4 <- tested and best compatibility
 *later i will add this to inGame Menu
 *you have to adjust the Snes9x4C_Net.gpu and rename to Snes9x4C.gpu

ver 20100429
------------
FX1 support added
-Star Wing/Star Fox
-Dirt Trax FX
FX2 support added
-Super Mario World 2 - Yoshi's Island
-Winter Gold
-Doom (slow, unplayable)
OBC1 support added
-Metal Combat: Falcon's Revenge
DSP-2 suport added
-Dungeon Master
DSP-3
-SD Gundam GX --bugged?
DSP-4
-Top Gear 3000
other that work now:
-Super Mario All-Stars
-The Lion King
-Sunset Riders

+autodetection of HighRes Games added
+some other Bugs fixed

For Fx Title use 400Mhz, Frameskip to 60/3 and the NoSound commandline option "-ns" !

ver 20100407
------------
+C4 support added
+fullscreen commandline option bug solved (use -fs for fullscreen)

ver 20100330
------------
+renamed from Snes9x to Snes9x4D (for Dingux)
+new Snes9x4D Icon added
+Fullscreen Scaling added again for Normal Mode
+Autodetect Highres Games (Secret of Mana, Seiken 3 & Rudra no Hihou) and switch automatically to HighRes Mode
+Scaling is not available in HighRes Mode (Menu-Option inactive)

ver 20100315
+SA-1 Support added (Super Mario RPG, and all other SA-1 Games are working!!!)
(Savestates of SA-1 Games are untested (better only use InGames saves!)

+"-nohi" or "-nohires" commandline option added
+"-hi" or "-hires" commandline option added
+transparency Option removed, doesn't affect speed
+memory leak in Menu fixed

ver 20100313
------------
+screenshot feature fixed (in highres mode screenshots still are not fullscreen caps)

ver 20100301
------------
+Soul Blazer works now (i think only the german version was not working!?)

ver 20100224
------------
+Transparency Menu Option added
+Zipped ROMs Support
+HighRes Support enabled (solves Secret of Mana, Seiken 3 & Rudra no Hihou font problems)

ver 20100221
------------
+support for SDD1 Emu added (GraphicPack NOT needed anymore!! -> removed)
+Street Fighter Alpha 2 & Star Ocean (all work)

v1.39-ff
------------
-base source see changes.txt


=======
Credits
=======
Created by the Snes9x team.

Port to libSDL by:
* SiENcE
  crankgaming.blogspot.com
  regards to joyrider & g17

Pandora port started and initially created by:
* skeezix

Further work and maintenance:
* Ivanovic

Special thanks to:
* EvilDragon (picklelauncher theme)
* john4p (support for cutting borders in HW-accel mode)
* Notaz (modified libSDL for Pandora allowing HW-accelerated scaling)
* pickle (picklelauncher, Scale2x)
* WizardStan (smooth scaler)
* every user reporting issues and providing feedback!


==================================================================
KNOW ISSUES (possibly outdated or not valid for the Pandora port!)
==================================================================
-known Games having issues:
FX1
*Stunt Race FX (Wild Trax) (graphic glitches)
*Dirt Racer (don't work)
*Vortex (don't work)
FX2
*Star Fox 2 final (graphic glitches)
*Doom (slow as hell)


===========================================================
Some words from the original porter of Snes9x to use libsdl
===========================================================
Snes9x4X v20101010
====================
(c)^SiENcE^2010.10.10

I renamed Snes9x4D to Snes9x4X. Every System gets his own Name extension.

Supported devices:
 Snes9x4C	- GPH Caanoo
 Snes9x4Wiz	- GPH Wiz
 Snes9x4D	- Dingoo
 Snes9x4P	- Pandora
 Snes9x4W	- Windows

Blog:
http://crankgaming.blogspot.com/

Sources: (of the "initial" port up to version "1.39ff.20111207.1")
svn://zwischenwelt.org:20684/dingoo/Snes9x-sdl-1.39ff

Hint: I removed a speedhack from old 1.39 source, just because it only fakes fps.

Original port taken from:
http://www9.atpages.jp/~drowsnug95/?p=252
http://www9.atpages.jp/~drowsnug95/wp-content/uploads/2009/10/snes9x_src.zip

Contact:
seabeams at gmx.de

^SiENcE
