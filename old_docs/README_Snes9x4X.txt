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

KNOW ISSUES:
-No Fullscreen support for HighRes Games
-known Games having issues:
FX1
*Stunt Race FX (Wild Trax) (graphic glitches)
*Dirt Racer (don't work)
*Vortex (don't work)
FX2
*Star Fox 2 final (graphic glitches)
*Doom (slow as hell)

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

Blog:
http://crankgaming.blogspot.com/

Sources:
svn://zwischenwelt.org:20684/dingoo/Snes9x-sdl-1.39ff

Hint: I removed a speedhack from old 1.39 source, just because it only fakes fps.

Original port taken from:
http://www9.atpages.jp/~drowsnug95/?p=252
http://www9.atpages.jp/~drowsnug95/wp-content/uploads/2009/10/snes9x_src.zip

Contact:
seabeams at gmx.de

^SiENcE