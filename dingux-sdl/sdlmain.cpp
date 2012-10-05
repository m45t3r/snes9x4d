/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co., Limited and its subsidiary companies.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <ctype.h>
#include <SDL/SDL.h>
#include <time.h>
#include "sdlmenu.h"
#include "keydef.h"
#include "scaler.h"

#ifdef NETPLAY_SUPPORT
	#include "../netplay.h"
#endif

#ifdef CAANOO
	#include "caanoo.h"
#else
	#include "dingoo.h"
#endif

#include "snes9x.h"
#include "memmap.h"
#include "debug.h"
#include "cpuexec.h"
#include "ppu.h"
#include "snapshot.h"
#include "apu.h"
#include "display.h"
#include "gfx.h"
#include "soundux.h"
#include "spc700.h"

#ifdef WIN32
static void sync() { }
#endif

#ifdef CAANOO
	SDL_Joystick* keyssnes;
#else
	uint8 *keyssnes;
#endif

#ifdef NETPLAY_SUPPORT
	static uint32	joypads[8];
	static uint32	old_joypads[8];
#endif

// SaveSlotNumber
char SaveSlotNum = 0;

bool8_32 Scale = FALSE;
char msg[256];
short vol=50;
static int mixerdev = 0;
clock_t start;

const char *GetHomeDirectory();
void OutOfMemory();

extern void S9xDisplayFrameRate (uint8 *, uint32);
extern void S9xDisplayString (const char *string, uint8 *, uint32, int);
extern SDL_Surface *screen, *gfxscreen;

static uint32 ffc = 0;
uint32 xs = 320; //width
uint32 ys = 240; //height
uint32 cl = 12; //ypos in highres mode
uint32 cs = 0;
uint32 mfs = 10; //skippedframes

char *rom_filename = NULL;
char *snapshot_filename = NULL;

#ifndef _ZAURUS
#if defined(__linux) || defined(__sun)
static void sigbrkhandler(int)
{
#ifdef DEBUGGER
	CPU.Flags |= DEBUG_MODE_FLAG;
	signal(SIGINT, (SIG_PF) sigbrkhandler);
#endif
}
#endif
#endif

void S9xParseArg (char **argv, int &i, int argc)
{

	if (strcasecmp (argv [i], "-b") == 0 ||
		strcasecmp (argv [i], "-bs") == 0 ||
		strcasecmp (argv [i], "-buffersize") == 0)
	{
	if (i + 1 < argc)
		Settings.SoundBufferSize = atoi (argv [++i]);
	else
		S9xUsage ();
	}
	else if (strcmp (argv [i], "-l") == 0 ||
		 strcasecmp (argv [i], "-loadsnapshot") == 0)
	{
	if (i + 1 < argc)
		snapshot_filename = argv [++i];
	else
		S9xUsage ();
	}
	else if (strcmp (argv [i], "-xs") == 0)
	{
	if (i + 1 < argc)
		xs = atoi(argv [++i]);
	else
		S9xUsage ();
	}
	else if (strcmp (argv [i], "-ys") == 0)
	{
	if (i + 1 < argc)
		ys = atoi(argv [++i]);
	else
		S9xUsage ();
	}
	else if (strcmp (argv [i], "-cl") == 0)
	{
	if (i + 1 < argc)
		cl = atoi(argv [++i]);
	else
		S9xUsage ();
	}
	else if (strcmp (argv [i], "-cs") == 0)
	{
	if (i + 1 < argc)
		cs = atoi(argv [++i]);
	else
		S9xUsage ();
	}
	else if (strcmp (argv [i], "-mfs") == 0)
	{
	if (i + 1 < argc)
		mfs = atoi(argv [++i]);
	else
		S9xUsage ();
	}
	else
		S9xUsage ();
}

const char *S9xGetCfgName()
{
	static char filename [PATH_MAX + 1];
	char drive [_MAX_DRIVE + 1];
	char dir [_MAX_DIR + 1];
	char fname [_MAX_FNAME + 1];
	char ext [_MAX_EXT + 1];

	_splitpath (rom_filename, drive, dir, fname, ext);
	sprintf(filename, "%s/.snes9x4d/%s.cfg", GetHomeDirectory(), fname);

	return (filename);
}

int is_home_created()
{
	char home[512];
	sprintf(home, "%s/%s", GetHomeDirectory(), ".snes9x4d");
	mkdir (home
	#ifndef WIN32
	, 0777
	#endif
	);
	if(!errno) return FALSE;
	return TRUE;
}

void S9xWriteConfig()
{
	FILE *fp;

	if(!rom_filename) return;
	if(!is_home_created()) return;

	fp = fopen(S9xGetCfgName(), "wb");
	if(!fp) return;
	fwrite(&Settings, 1, sizeof(Settings), fp);
	fwrite(&Scale, 1, sizeof(Scale), fp);
	fclose(fp);
}

void S9xReadConfig()
{
	FILE *fp;

	if(!rom_filename) return;
	if(!is_home_created()) return;

	fp = fopen(S9xGetCfgName(), "rb");
	if(!fp) {
		S9xWriteConfig();
		return;
	}
	fread(&Settings, 1, sizeof(Settings), fp);
	fread(&Scale, 1, sizeof(Scale), fp);
	fclose(fp);
}

extern "C"
#undef main
int main (int argc, char **argv)
{
	start = clock();
	if (argc < 2) {
		S9xUsage ();
	}

	ZeroMemory (&Settings, sizeof (Settings));

	Settings.JoystickEnabled = FALSE;
	Settings.SoundPlaybackRate = 3;
	Settings.Stereo = TRUE;
	Settings.SoundSync = TRUE;
	Settings.SoundBufferSize = 256;
	Settings.CyclesPercentage = 100;
	Settings.DisableSoundEcho = FALSE;
	Settings.APUEnabled = Settings.NextAPUEnabled = TRUE;
	Settings.InterpolatedSound = TRUE;
	Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
#ifdef PANDORA
	Settings.SkipFrames = 1;
#else
	Settings.SkipFrames = AUTO_FRAMERATE;
#endif
	Settings.DisplayFrameRate = FALSE;
	Settings.ShutdownMaster = TRUE;
	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;
	Settings.FrameTime = Settings.FrameTimeNTSC;
	Settings.DisableSampleCaching = FALSE;
	Settings.DisableMasterVolume = FALSE;
	Settings.Mouse = FALSE;	//TRUE
	Settings.SuperScope = FALSE;
	Settings.MultiPlayer5 = FALSE;
	Settings.ControllerOption = SNES_MULTIPLAYER5;
	Settings.ControllerOption = 0;
	Settings.Transparency = FALSE; //TRUE;
	Settings.SixteenBit = TRUE;
	Settings.SupportHiRes = FALSE; //autodetected for known highres roms
	Settings.NetPlay = FALSE;
	Settings.ServerName [0] = 0;
	Settings.ThreadSound = FALSE;
	Settings.AutoSaveDelay = 30;
	Settings.ApplyCheats = TRUE;
	Settings.TurboMode = FALSE;
	Settings.TurboSkipFrames = 15;
	if (Settings.ForceNoTransparency)
		Settings.Transparency = FALSE;
	if (Settings.Transparency)
		Settings.SixteenBit = TRUE;

	//parse commandline arguments for ROM filename
	rom_filename = S9xParseArgs (argv, argc);
	printf("Rom filename: %s\n", rom_filename);

	S9xReadConfig();

	Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;

	if (!Memory.Init () || !S9xInitAPU())
	{
		OutOfMemory ();
	}

   (void) S9xInitSound (Settings.SoundPlaybackRate, Settings.Stereo, Settings.SoundBufferSize);

	if (!Settings.APUEnabled)
		S9xSetSoundMute (TRUE);

	uint32 saved_flags = CPU.Flags;

#ifdef GFX_MULTI_FORMAT
	S9xSetRenderPixelFormat (RGB565);
#endif

#ifndef DINGOO
	// ROM selector if no rom filename is available!!!!!!!!!!!!!!
	if (!rom_filename)
	{
		S9xInitDisplay (argc, argv);
		if (!S9xGraphicsInit ())
		{
			OutOfMemory ();
		}
		S9xInitInputDevices ();

		// just to init Font here for ROM selector
		S9xReset ();

		do
		{
			rom_filename = menu_romselector();
		} while(rom_filename==NULL);

		S9xDeinitDisplay();
		printf ("Romfile selected: %s\n", rom_filename);
	}
#endif

	if (rom_filename)
	{
		if (!Memory.LoadROM (rom_filename))
		{
			char dir [_MAX_DIR + 1];
			char drive [_MAX_DRIVE + 1];
			char name [_MAX_FNAME + 1];
			char ext [_MAX_EXT + 1];
			char fname [_MAX_PATH + 1];

			_splitpath (rom_filename, drive, dir, name, ext);
			_makepath (fname, drive, dir, name, ext);

			strcpy (fname, S9xGetROMDirectory ());
			strcat (fname, SLASH_STR);
			strcat (fname, name);

			if (ext [0])
			{
				strcat (fname, ".");
				strcat (fname, ext);
			}
			_splitpath (fname, drive, dir, name, ext);
			_makepath (fname, drive, dir, name, ext);

			if (!Memory.LoadROM (fname))
			{
				printf ("Error opening: %s\n", rom_filename);
				OutOfMemory();
			}
		}
		Memory.LoadSRAM (S9xGetFilename (".srm"));
	}
	else
	{
		S9xExit();
	}

	S9xInitDisplay (argc, argv);
	if (!S9xGraphicsInit ())
	{
		OutOfMemory ();
	}

	S9xInitInputDevices ();

	CPU.Flags = saved_flags;
	Settings.StopEmulation = FALSE;

#ifdef DEBUGGER
	struct sigaction sa;
	sa.sa_handler = sigbrkhandler;
#ifdef SA_RESTART
	sa.sa_flags = SA_RESTART;
#else
	sa.sa_flags = 0;
#endif
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
#endif

#ifdef NETPLAY_SUPPORT
	if (strlen(Settings.ServerName) == 0)
	{
		char	*server = getenv("S9XSERVER");
		if (server)
		{
			strncpy(Settings.ServerName, server, 127);
			Settings.ServerName[127] = 0;
		}
	}

	char	*port = getenv("S9XPORT");
	if (Settings.Port >= 0 && port)
		Settings.Port = atoi(port);
	else
	if (Settings.Port < 0)
		Settings.Port = -Settings.Port;

	if (Settings.NetPlay)
	{
		NetPlay.MaxFrameSkip = 10;

		if (!S9xNPConnectToServer(Settings.ServerName, Settings.Port, Memory.ROMName))
		{
			fprintf(stderr, "Failed to connect to server %s on port %d.\n", Settings.ServerName, Settings.Port);
			S9xExit();
		}

		fprintf(stderr, "Connected to server %s on port %d as player #%d playing %s.\n", Settings.ServerName, Settings.Port, NetPlay.Player, Memory.ROMName);
	}
#endif

	//Handheld Key Infos
#ifdef CAANOO
	sprintf(msg,"Press HOME to Show MENU");
#elif CYGWIN32
	sprintf(msg,"Press ESC+LALT to Show MENU");
#else
	sprintf(msg,"Press SELECT+START to Show MENU");
#endif
	S9xSetInfoString(msg);

	//load Snapshot
	if (snapshot_filename)
	{
		int Flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
		if (!S9xLoadSnapshot (snapshot_filename))
			exit (1);
		CPU.Flags |= Flags;
	}

#ifndef _ZAURUS
	S9xGraphicsMode ();
	sprintf (String, "\"%s\" %s: %s", Memory.ROMName, TITLE, VERSION);
	S9xSetTitle (String);
#endif

#ifdef JOYSTICK_SUPPORT
	uint32	JoypadSkip = 0;
#endif

	S9xSetSoundMute(FALSE);

#ifdef NETPLAY_SUPPORT
	bool8	NP_Activated = Settings.NetPlay;
#endif

	while (1)
	{
#ifdef NETPLAY_SUPPORT
		if (NP_Activated)
		{
			if (NetPlay.PendingWait4Sync && !S9xNPWaitForHeartBeatDelay(100))
			{
				S9xProcessEvents(FALSE);
				continue;
			}

//			for (int J = 0; J < 8; J++)
//				old_joypads[J] = MovieGetJoypad(J);

//			for (int J = 0; J < 8; J++)
//				MovieSetJoypad(J, joypads[J]);

			if (NetPlay.Connected)
			{
				if (NetPlay.PendingWait4Sync)
				{
					NetPlay.PendingWait4Sync = FALSE;
					NetPlay.FrameCount++;
					S9xNPStepJoypadHistory();
				}
			}
			else
			{
				fprintf(stderr, "Lost connection to server.\n");
				S9xExit();
			}
		}
#endif

#ifdef DEBUGGER
		if (!Settings.Paused || (CPU.Flags & (DEBUG_MODE_FLAG | SINGLE_STEP_FLAG)))
#else
		if (!Settings.Paused)
#endif
			S9xMainLoop();

#ifdef NETPLAY_SUPPORT
		if (NP_Activated)
		{
//			for (int J = 0; J < 8; J++)
//				MovieSetJoypad(J, old_joypads[J]);
		}
#endif

#ifdef DEBUGGER
		if (Settings.Paused || (CPU.Flags & DEBUG_MODE_FLAG))
#else
		if (Settings.Paused)
#endif
			S9xSetSoundMute(TRUE);

#ifdef DEBUGGER
		if (CPU.Flags & DEBUG_MODE_FLAG)
			S9xDoDebug();
		else
#endif
		if (Settings.Paused)
		{
			S9xProcessEvents(FALSE);
			usleep(100000);
		}

#ifdef JOYSTICK_SUPPORT
		if (unixSettings.JoystickEnabled && (JoypadSkip++ & 1) == 0)
			ReadJoysticks();
#endif

		S9xProcessEvents(TRUE);

#ifdef DEBUGGER
		if (!Settings.Paused && !(CPU.Flags & DEBUG_MODE_FLAG))
#else
		if (!Settings.Paused)
#endif
			S9xSetSoundMute(FALSE);
	}

	return (0);
}

void S9xAutoSaveSRAM ()
{
	Memory.SaveSRAM (S9xGetFilename (".srm"));
}

void OutOfMemory()
{
	fprintf (stderr, "Snes9X: Memory allocation failure - not enough RAM/virtual memory available.\n S9xExiting...\n");

	Memory.Deinit ();
	S9xDeinitAPU ();
	S9xDeinitDisplay();

	exit (1);
}

void S9xExit()
{
	S9xSetSoundMute(true);

#ifdef NETPLAY_SUPPORT
	if (Settings.NetPlay)
		S9xNPDisconnect();
#endif

	S9xWriteConfig();
	Memory.SaveSRAM (S9xGetFilename (".srm"));
	//S9xSaveCheatFile (S9xGetFilename (".cht")); // not needed for embedded devices

	Memory.Deinit ();
	S9xDeinitAPU ();
	S9xDeinitDisplay ();

	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();

	exit (0);
}

Uint16 sfc_key[256];
void S9xInitInputDevices ()
{
#ifdef CAANOO
	keyssnes = SDL_JoystickOpen(0);
#else
	keyssnes = SDL_GetKeyState(NULL);
#endif

	memset(sfc_key, 0, 256);

#ifdef CAANOO
	// Caanoo mapping
	sfc_key[A_1] = CAANOO_BUTTON_B;	//Snes A
	sfc_key[B_1] = CAANOO_BUTTON_X;	//Snes B
	sfc_key[X_1] = CAANOO_BUTTON_Y;	//Snes X
	sfc_key[Y_1] = CAANOO_BUTTON_A;	//Snes Y
	sfc_key[L_1] = CAANOO_BUTTON_L;
	sfc_key[R_1] = CAANOO_BUTTON_R;
	sfc_key[START_1] = CAANOO_BUTTON_HELP1;
	sfc_key[SELECT_1] = CAANOO_BUTTON_HELP2;
	sfc_key[LEFT_1] = CAANOO_BUTTON_LEFT;
	sfc_key[RIGHT_1] = CAANOO_BUTTON_RIGHT;
	sfc_key[UP_1] = CAANOO_BUTTON_UP;
	sfc_key[DOWN_1] = CAANOO_BUTTON_DOWN;

	sfc_key[QUIT] = CAANOO_BUTTON_HOME;
#else
	// Dingoo mapping
	sfc_key[A_1] = DINGOO_BUTTON_A;
	sfc_key[B_1] = DINGOO_BUTTON_B;
	sfc_key[X_1] = DINGOO_BUTTON_X;
	sfc_key[Y_1] = DINGOO_BUTTON_Y;
	sfc_key[L_1] = DINGOO_BUTTON_L;
	sfc_key[R_1] = DINGOO_BUTTON_R;
	sfc_key[START_1] = DINGOO_BUTTON_START;
	sfc_key[SELECT_1] = DINGOO_BUTTON_SELECT;
	sfc_key[LEFT_1] = DINGOO_BUTTON_LEFT;
	sfc_key[RIGHT_1] = DINGOO_BUTTON_RIGHT;
	sfc_key[UP_1] = DINGOO_BUTTON_UP;
	sfc_key[DOWN_1] = DINGOO_BUTTON_DOWN;
#endif

	int i = 0;
	char *envp, *j;
	envp = j = getenv ("S9XKEYS");
	if (envp) {
		do {
			if (j = strchr(envp, ','))
				*j = 0;
			if (i == 0) sfc_key[QUIT] = atoi(envp);
			else if (i == 1)  sfc_key[A_1] = atoi(envp);
			else if (i == 2)  sfc_key[B_1] = atoi(envp);
			else if (i == 3)  sfc_key[X_1] = atoi(envp);
			else if (i == 4)  sfc_key[Y_1] = atoi(envp);
			else if (i == 5)  sfc_key[L_1] = atoi(envp);
			else if (i == 6)  sfc_key[R_1] = atoi(envp);
			else if (i == 7)  sfc_key[START_1] = atoi(envp);
			else if (i == 8)  sfc_key[SELECT_1] = atoi(envp);
			else if (i == 9)  sfc_key[LEFT_1] = atoi(envp);
			else if (i == 10) sfc_key[RIGHT_1] = atoi(envp);
			else if (i == 11) sfc_key[UP_1] = atoi(envp);
			else if (i == 12) sfc_key[DOWN_1] = atoi(envp);
/*			else if (i == 13) sfc_key[LU_2] = atoi(envp);
			else if (i == 14) sfc_key[LD_2] = atoi(envp);
			else if (i == 15) sfc_key[RU_2] = atoi(envp);
			else if (i == 16) sfc_key[RD_2] = atoi(envp);
*/			envp = j + 1;
			++i;
		} while(j);
	}

}

const char *GetHomeDirectory ()
{
#if CAANOO
	return (".");
#elif CYGWIN32
	return (".");
#elif WIN32
	return (".");
#else
	return (getenv ("HOME"));
#endif
}

const char *S9xGetSnapshotDirectory ()
{
	static char filename [PATH_MAX];
	const char *snapshot;

	if (!(snapshot = getenv ("SNES9X_SNAPSHOT_DIR")) &&
	!(snapshot = getenv ("SNES96_SNAPSHOT_DIR")))
	{
		const char *home = GetHomeDirectory ();
		strcpy (filename, home);
		strcat (filename, SLASH_STR);
		strcat (filename, ".snes96_snapshots");
		mkdir (filename
		#ifndef WIN32
		, 0777
		#endif
		);
		//chown (filename, getuid (), getgid ());
	}
	else
	return (snapshot);

	return (filename);
}

const char *S9xGetFilename (const char *ex)
{
	static char filename [PATH_MAX + 1];
	char drive [_MAX_DRIVE + 1];
	char dir [_MAX_DIR + 1];
	char fname [_MAX_FNAME + 1];
	char ext [_MAX_EXT + 1];

	_splitpath (Memory.ROMFilename, drive, dir, fname, ext);
	strcpy (filename, S9xGetSnapshotDirectory ());
	strcat (filename, SLASH_STR);
	strcat (filename, fname);
	strcat (filename, ex);

	return (filename);
}

const char *S9xGetROMDirectory ()
{
	const char *roms;

	if (!(roms = getenv ("SNES9X_ROM_DIR")) && !(roms = getenv ("SNES96_ROM_DIR")))
		return ("." SLASH_STR "roms");
	else
		return (roms);
}

const char *S9xBasename (const char *f)
{
	const char *p;
	if ((p = strrchr (f, '/')) != NULL || (p = strrchr (f, '\\')) != NULL)
	return (p + 1);

	return (f);
}

bool8 S9xOpenSnapshotFile (const char *fname, bool8 read_only, STREAM *file)
{
	char filename [PATH_MAX + 1];
	char drive [_MAX_DRIVE + 1];
	char dir [_MAX_DIR + 1];
	char ext [_MAX_EXT + 1];

	_splitpath (fname, drive, dir, filename, ext);

	if (*drive || *dir == '/' ||
	(*dir == '.' && (*(dir + 1) == '/'
		)))
	{
	strcpy (filename, fname);
	if (!*ext)
		strcat (filename, ".s96");
	}
	else
	{
	strcpy (filename, S9xGetSnapshotDirectory ());
	strcat (filename, SLASH_STR);
	strcat (filename, fname);
	if (!*ext)
		strcat (filename, ".s96");
	}

#ifdef ZLIB
	if (read_only)
	{
	if ((*file = OPEN_STREAM (filename, "rb")))
		return (TRUE);
	}
	else
	{
	if ((*file = OPEN_STREAM (filename, "wb")))
	{
		//chown (filename, getuid (), getgid ());
		return (TRUE);
	}
	}
#else
	char command [PATH_MAX];

	if (read_only)
	{
	sprintf (command, "gzip -d <\"%s\"", filename);
	if (*file = popen (command, "r"))
		return (TRUE);
	}
	else
	{
	sprintf (command, "gzip --best >\"%s\"", filename);
	if (*file = popen (command, "wb"))
		return (TRUE);
	}
#endif
	return (FALSE);
}

void S9xCloseSnapshotFile (STREAM file)
{
#ifdef ZLIB
	CLOSE_STREAM (file);
#else
	pclose (file);
#endif
	sync();
}

bool8_32 S9xInitUpdate ()
{
	return (TRUE);
}

bool8_32 S9xDeinitUpdate (int Width, int Height)
{
	uint32 spd = (Settings.SupportHiRes ? 256/2 : 0);
	uint32 dpd = (screen->w - 256) / 2;
	uint32 dpo = (screen->w - 256)/4 + (screen->h - 224)/2*screen->w/2;

	SDL_LockSurface(screen);

	if (Settings.SupportHiRes)
	{
		if (Width > 256)
		{
			// If SupportHiRes is active and HighRes Frame
			uint16 *dp16 = (uint16 *)(screen->pixels) + dpo*2;
			uint32 *sp32 = (uint32 *)(GFX.Screen);
			for(int y = 224; y--;) {
				for(int x = 256; x--; ) {
					*dp16++ = *sp32++;
				}
				dp16 += dpd*2;
			}
		}
		else
		{
			if(Scale) {
				// put here upscale to 400x240 and 480x272
				(*upscale_p)((uint32_t *)screen->pixels, (uint32_t *)GFX.Screen, 512); 
			} else goto __jump;
		}
	}
	else
	{
		// if scaling for non-highres (is centered)
		if(Scale) {
			// put here upscale to 400x240 and 480x272
			(*upscale_p)((uint32_t *)screen->pixels, (uint32_t *)GFX.Screen, 256); 

		} else {
		__jump:
			uint32 *dp32 = (uint32 *)(screen->pixels) + dpo;
			uint32 *sp32 = (uint32 *)(GFX.Screen);
			for(int y = 224; y--;) {
				for(int x = 256/2; x--; ) {
					*dp32++ = *sp32++;
				}
				sp32 += spd;
				dp32 += dpd;
			}
		}
	}

	if (GFX.InfoString)
		S9xDisplayString (GFX.InfoString, (uint8 *)screen->pixels + screen->w - 256, screen->pitch, 0);
	else if (Settings.DisplayFrameRate)
		S9xDisplayFrameRate ((uint8 *)screen->pixels + screen->w - 256, screen->pitch);

	SDL_UnlockSurface(screen);
	SDL_Flip(screen);
	return(TRUE);
}

#ifndef _ZAURUS
static unsigned long now ()
{
	static unsigned long seconds_base = 0;
	struct timeval tp;
	gettimeofday (&tp, NULL);
	if (!seconds_base)
	seconds_base = tp.tv_sec;

	return ((tp.tv_sec - seconds_base) * 1000 + tp.tv_usec / 1000);
}
#endif

void _makepath (char *path, const char *, const char *dir,
		const char *fname, const char *ext)
{
	if (dir && *dir)
	{
	strcpy (path, dir);
	strcat (path, "/");
	}
	else
	*path = 0;
	strcat (path, fname);
	if (ext && *ext)
	{
		strcat (path, ".");
		strcat (path, ext);
	}
}

void _splitpath (const char *path, char *drive, char *dir, char *fname,
		 char *ext)
{
	*drive = 0;

	char *slash = strrchr ( (char*) path, '/');
	if (!slash)
	slash = strrchr ( (char*) path, '\\');

	char *dot = strrchr ( (char*) path, '.');

	if (dot && slash && dot < slash)
	dot = NULL;

	if (!slash)
	{
	strcpy (dir, "");
	strcpy (fname, path);
		if (dot)
		{
		*(fname + (dot - path)) = 0;
		strcpy (ext, dot + 1);
		}
	else
		strcpy (ext, "");
	}
	else
	{
	strcpy (dir, path);
	*(dir + (slash - path)) = 0;
	strcpy (fname, slash + 1);
		if (dot)
	{
		*(fname + (dot - slash) - 1) = 0;
			strcpy (ext, dot + 1);
	}
	else
		strcpy (ext, "");
	}
}

#ifndef _ZAURUS
void S9xToggleSoundChannel (int c)
{
	if (c == 8)
	so.sound_switch = 255;
	else
	so.sound_switch ^= 1 << c;
	S9xSetSoundControl (so.sound_switch);
}
#endif

void S9xSyncSpeed () // called from S9xMainLoop in ../cpuexec.cpp
{
#ifdef NETPLAY_SUPPORT
	if (Settings.NetPlay && NetPlay.Connected)
	{
	#if defined(NP_DEBUG) && NP_DEBUG == 2
		printf("CLIENT: SyncSpeed @%d\n", S9xGetMilliTime());
	#endif

		S9xNPSendJoypadUpdate(old_joypads[0]);
		for (int J = 0; J < 8; J++)
			joypads[J] = S9xNPGetJoypad(J);

		if (!S9xNPCheckForHeartBeat())
		{
			NetPlay.PendingWait4Sync = !S9xNPWaitForHeartBeatDelay(100);
		#if defined(NP_DEBUG) && NP_DEBUG == 2
			if (NetPlay.PendingWait4Sync)
				printf("CLIENT: PendingWait4Sync1 @%d\n", S9xGetMilliTime());
		#endif

			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
		}
		else
		{
			NetPlay.PendingWait4Sync = !S9xNPWaitForHeartBeatDelay(200);
		#if defined(NP_DEBUG) && NP_DEBUG == 2
			if (NetPlay.PendingWait4Sync)
				printf("CLIENT: PendingWait4Sync2 @%d\n", S9xGetMilliTime());
		#endif

			if (IPPU.SkippedFrames < NetPlay.MaxFrameSkip)
			{
				IPPU.RenderThisFrame = FALSE;
				IPPU.SkippedFrames++;
			}
			else
			{
				IPPU.RenderThisFrame = TRUE;
				IPPU.SkippedFrames = 0;
			}
		}

		if (!NetPlay.PendingWait4Sync)
		{
			NetPlay.FrameCount++;
			S9xNPStepJoypadHistory();
		}

		return;
	}
#endif
	if (!Settings.TurboMode && Settings.SkipFrames == AUTO_FRAMERATE)
	{
		static struct timeval next1 = {0, 0};
		struct timeval now;

		while (gettimeofday (&now, NULL) < 0) ;
		if (next1.tv_sec == 0)
		{
			next1 = now;
			next1.tv_usec++;
		}

		if (timercmp(&next1, &now, >))
		{
			if (IPPU.SkippedFrames == 0)
			{
				do
				{
					CHECK_SOUND ();
					while (gettimeofday (&now, NULL) < 0) ;
				} while (timercmp(&next1, &now, >));
			}
			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
		}
		else
		{
			if (IPPU.SkippedFrames < mfs)
			{
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
			}
			else
			{
			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
			next1 = now;
			}
		}
		next1.tv_usec += Settings.FrameTime;
		if (next1.tv_usec >= 1000000)
		{
			next1.tv_sec += next1.tv_usec / 1000000;
			next1.tv_usec %= 1000000;
		}
	}
	else
	{
		if (++IPPU.FrameSkip >= (Settings.TurboMode ? Settings.TurboSkipFrames
								: Settings.SkipFrames))
		{
			IPPU.FrameSkip = 0;
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		}
		else
		{
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
		}
	}
}

void S9xProcessEvents (bool8_32 block)
{
	SDL_Event event;

	while(block && SDL_PollEvent(&event))
	{
		switch(event.type)
		{
#ifdef CAANOO
			// CAANOO -------------------------------------------------------------
			case SDL_JOYBUTTONDOWN:
				keyssnes = SDL_JoystickOpen(0);
				//QUIT Emulator
				if ( SDL_JoystickGetButton(keyssnes, sfc_key[QUIT]) && SDL_JoystickGetButton(keyssnes, sfc_key[B_1] ) )
				{
					S9xExit();
				}
				// MAINMENU
				else if ( SDL_JoystickGetButton(keyssnes, sfc_key[QUIT]) )
				{
					S9xSetSoundMute (TRUE);
					menu_loop();
					S9xSetSoundMute(FALSE);
				}
				break;

			case SDL_JOYBUTTONUP:
				keyssnes = SDL_JoystickOpen(0);
				switch(event.jbutton.button)
				{
				}
				break;
#else
			//DINGOO ------------------------------------------------------
			case SDL_KEYDOWN:
				keyssnes = SDL_GetKeyState(NULL);

				//QUIT Emulator
				if ( (keyssnes[sfc_key[SELECT_1]] == SDL_PRESSED) &&(keyssnes[sfc_key[START_1]] == SDL_PRESSED) && (keyssnes[sfc_key[X_1]] == SDL_PRESSED) )
				{
					S9xExit();
				}
				//RESET ROM Playback
				else if ((keyssnes[sfc_key[SELECT_1]] == SDL_PRESSED) && (keyssnes[sfc_key[START_1]] == SDL_PRESSED) && (keyssnes[sfc_key[B_1]] == SDL_PRESSED))
				{
					S9xReset();
				}
				//SAVE State
				else if ( (keyssnes[sfc_key[START_1]] == SDL_PRESSED) && (keyssnes[sfc_key[R_1]] == SDL_PRESSED) )
				{
					//extern char snapscreen;
					char fname[256], ext[20];
					S9xSetSoundMute(true);
					sprintf(ext, ".00%d", SaveSlotNum);
					strcpy(fname, S9xGetFilename (ext));
					S9xFreezeGame (fname);
					capt_screenshot();
					sprintf(ext, ".s0%d", SaveSlotNum);
					strcpy(fname, S9xGetFilename (ext));
					save_screenshot(fname);
					S9xSetSoundMute(false);
				}
				//LOAD State
				else if ( (keyssnes[sfc_key[START_1]] == SDL_PRESSED) && (keyssnes[sfc_key[L_1]] == SDL_PRESSED) )
				{
					char fname[256], ext[8];
					S9xSetSoundMute(true);
					sprintf(ext, ".00%d", SaveSlotNum);
					strcpy(fname, S9xGetFilename (ext));
					S9xLoadSnapshot (fname);
					S9xSetSoundMute(false);
				}
				// MAINMENU
				else if ((keyssnes[sfc_key[SELECT_1]] == SDL_PRESSED)&&(keyssnes[sfc_key[START_1]] == SDL_PRESSED) )
				{
					S9xSetSoundMute(true);
					menu_loop();
					S9xSetSoundMute(false);
				}
				break;
			case SDL_KEYUP:
				keyssnes = SDL_GetKeyState(NULL);
				break;
#endif //CAANOO
		}
	}
}

//#endif

static long log2 (long num)
{
	long n = 0;

	while (num >>= 1)
	n++;

	return (n);
}

uint32 S9xReadJoypad (int which1)
{
	uint32 val=0x80000000;

	if (which1 > 4)
		return 0;

#ifdef CAANOO
	//player1
	if (SDL_JoystickGetButton(keyssnes, sfc_key[L_1]))			val |= SNES_TL_MASK;
	if (SDL_JoystickGetButton(keyssnes, sfc_key[R_1]))			val |= SNES_TR_MASK;
	if (SDL_JoystickGetButton(keyssnes, sfc_key[X_1]))			val |= SNES_X_MASK;
	if (SDL_JoystickGetButton(keyssnes, sfc_key[Y_1]))			val |= SNES_Y_MASK;
	if (SDL_JoystickGetButton(keyssnes, sfc_key[B_1]))			val |= SNES_B_MASK;
	if (SDL_JoystickGetButton(keyssnes, sfc_key[A_1]))			val |= SNES_A_MASK;
	if (SDL_JoystickGetButton(keyssnes, sfc_key[START_1]))		val |= SNES_START_MASK;
	if (SDL_JoystickGetButton(keyssnes, sfc_key[SELECT_1]))		val |= SNES_SELECT_MASK;
	if (SDL_JoystickGetAxis(keyssnes, 1) < -20000)				val |= SNES_UP_MASK;
	if (SDL_JoystickGetAxis(keyssnes, 1) > 20000)				val |= SNES_DOWN_MASK;
	if (SDL_JoystickGetAxis(keyssnes, 0) < -20000)				val |= SNES_LEFT_MASK;
	if (SDL_JoystickGetAxis(keyssnes, 0) > 20000)				val |= SNES_RIGHT_MASK;
#else
	//player1
	if (keyssnes[sfc_key[L_1]] == SDL_PRESSED)		val |= SNES_TL_MASK;
	if (keyssnes[sfc_key[R_1]] == SDL_PRESSED)		val |= SNES_TR_MASK;
	if (keyssnes[sfc_key[X_1]] == SDL_PRESSED)		val |= SNES_X_MASK;
	if (keyssnes[sfc_key[Y_1]] == SDL_PRESSED)		val |= SNES_Y_MASK;
	if (keyssnes[sfc_key[B_1]] == SDL_PRESSED)		val |= SNES_B_MASK;
	if (keyssnes[sfc_key[A_1]] == SDL_PRESSED)		val |= SNES_A_MASK;
	if (keyssnes[sfc_key[START_1]] == SDL_PRESSED)	val |= SNES_START_MASK;
	if (keyssnes[sfc_key[SELECT_1]] == SDL_PRESSED)	val |= SNES_SELECT_MASK;
	if (keyssnes[sfc_key[UP_1]] == SDL_PRESSED)		val |= SNES_UP_MASK;
	if (keyssnes[sfc_key[DOWN_1]] == SDL_PRESSED)	val |= SNES_DOWN_MASK;
	if (keyssnes[sfc_key[LEFT_1]] == SDL_PRESSED)	val |= SNES_LEFT_MASK;
	if (keyssnes[sfc_key[RIGHT_1]] == SDL_PRESSED)	val |= SNES_RIGHT_MASK;
	//player2
	/*
	if (keyssnes[sfc_key[UP_2]] == SDL_PRESSED)		val |= SNES_UP_MASK;
	if (keyssnes[sfc_key[DOWN_2]] == SDL_PRESSED)	val |= SNES_DOWN_MASK;
	if (keyssnes[sfc_key[LEFT_2]] == SDL_PRESSED)	val |= SNES_LEFT_MASK;
	if (keyssnes[sfc_key[RIGHT_2]] == SDL_PRESSED)	val |= SNES_RIGHT_MASK;
	if (keyssnes[sfc_key[LU_2]] == SDL_PRESSED)	val |= SNES_LEFT_MASK | SNES_UP_MASK;
	if (keyssnes[sfc_key[LD_2]] == SDL_PRESSED)	val |= SNES_LEFT_MASK | SNES_DOWN_MASK;
	if (keyssnes[sfc_key[RU_2]] == SDL_PRESSED)	val |= SNES_RIGHT_MASK | SNES_UP_MASK;
	if (keyssnes[sfc_key[RD_2]] == SDL_PRESSED)	val |= SNES_RIGHT_MASK | SNES_DOWN_MASK;
	*/
#endif

#ifdef NETPLAY_SUPPORT
	if (Settings.NetPlay)
		return (S9xNPGetJoypad (which1));
#endif

	return(val);
}
