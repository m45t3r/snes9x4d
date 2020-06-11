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

#include "snes9x.h"
#include "cheats.h"
#include "port.h"
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

#include "keydef.h"
#ifdef MIYOO
#include "miyoo.h"
#else
#include "dingoo.h"
#endif
#include "scaler.h"
#include "sdlmain.h"
#include "sdlmenu.h"
#include "sdlvideo.h"

// public global variables
short SaveSlotNum = 0;
bool8_32 Scale = FALSE;
#ifdef BILINEAR_SCALE
bool8_32 Bilinear = FALSE;
#endif
uint32 MaxAutoFrameSkip = 3;
uint16 sfc_key[256];

// private global variables
uint8 *keyssnes;
uint32 xs = SURFACE_WIDTH;  // width
uint32 ys = SURFACE_HEIGHT; // height
uint32 cl = 12;		    // ypos in highres mode
uint32 cs = 0;
char *rom_filename = NULL;
char *snapshot_filename = NULL;

#ifdef DEBUGGER
#if defined(__linux) || defined(__sun)
static void sigbrkhandler(int)
{
	CPU.Flags |= DEBUG_MODE_FLAG;
	signal(SIGINT, (SIG_PF)sigbrkhandler);
}
#endif
#endif

void S9xParseArg(char **argv, int &i, int argc)
{

	if (strcasecmp(argv[i], "-b") == 0 || strcasecmp(argv[i], "-bs") == 0 ||
	    strcasecmp(argv[i], "-buffersize") == 0) {
		if (i + 1 < argc)
			Settings.SoundBufferSize = atoi(argv[++i]);
		else
			S9xUsage();
	} else if (strcmp(argv[i], "-l") == 0 || strcasecmp(argv[i], "-loadsnapshot") == 0) {
		if (i + 1 < argc)
			snapshot_filename = argv[++i];
		else
			S9xUsage();
	} else if (strcmp(argv[i], "-xs") == 0) {
		if (i + 1 < argc)
			xs = atoi(argv[++i]);
		else
			S9xUsage();
	} else if (strcmp(argv[i], "-ys") == 0) {
		if (i + 1 < argc)
			ys = atoi(argv[++i]);
		else
			S9xUsage();
	} else if (strcmp(argv[i], "-cl") == 0) {
		if (i + 1 < argc)
			cl = atoi(argv[++i]);
		else
			S9xUsage();
	} else if (strcmp(argv[i], "-cs") == 0) {
		if (i + 1 < argc)
			cs = atoi(argv[++i]);
		else
			S9xUsage();
	} else if (strcmp(argv[i], "-mfs") == 0) {
		if (i + 1 < argc)
			MaxAutoFrameSkip = atoi(argv[++i]);
		else
			S9xUsage();
	} else
		S9xUsage();
}

const char *S9xGetCfgName()
{
	static char filename[PATH_MAX + 1];
	char drive[_MAX_DRIVE + 1];
	char dir[_MAX_DIR + 1];
	char fname[_MAX_FNAME + 1];
	char ext[_MAX_EXT + 1];

	_splitpath(rom_filename, drive, dir, fname, ext);
	sprintf(filename, "%s/" CFG_DIRECTORY "/%s.cfg", GetHomeDirectory(), fname);

	return (filename);
}

int is_home_created()
{
	char home[PATH_MAX + 1];
	sprintf(home, "%s/%s", GetHomeDirectory(), CFG_DIRECTORY);
	mkdir(home, 0777);
	if (!errno)
		return FALSE;
	return TRUE;
}

void S9xWriteConfig()
{
	FILE *fp;

	if (!rom_filename)
		return;
	if (!is_home_created())
		return;

	fp = fopen(S9xGetCfgName(), "wb");
	if (!fp)
		return;
	fwrite(&Settings, 1, sizeof(Settings), fp);
	fwrite(&Scale, 1, sizeof(Scale), fp);
#ifdef BILINEAR_SCALE
	fwrite(&Bilinear, 1, sizeof(Bilinear), fp);
#endif
	fwrite(&MaxAutoFrameSkip, 1, sizeof(MaxAutoFrameSkip), fp);
	fclose(fp);
}

void S9xReadConfig()
{
	FILE *fp;

	if (!rom_filename)
		return;
	if (!is_home_created())
		return;

	fp = fopen(S9xGetCfgName(), "rb");
	if (!fp) {
		S9xWriteConfig();
		return;
	}
	fread(&Settings, 1, sizeof(Settings), fp);
	fread(&Scale, 1, sizeof(Scale), fp);
#ifdef BILINEAR_SCALE
	fread(&Bilinear, 1, sizeof(Bilinear), fp);
#endif
	fread(&MaxAutoFrameSkip, 1, sizeof(MaxAutoFrameSkip), fp);
	fclose(fp);
}

void S9xResetConfig()
{
	ZeroMemory(&Settings, sizeof(Settings));

	Settings.JoystickEnabled = FALSE;
	Settings.SoundPlaybackRate = 5;
	Settings.Stereo = TRUE;
	Settings.SoundSync = TRUE;
	Settings.SoundBufferSize = 256;
	Settings.CyclesPercentage = 100;
	Settings.DisableSoundEcho = FALSE;
	Settings.APUEnabled = Settings.NextAPUEnabled = TRUE;
	Settings.InterpolatedSound = TRUE;
	Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
	Settings.SkipFrames = AUTO_FRAMERATE;
	Settings.DisplayFrameRate = FALSE;
	Settings.ShutdownMaster = TRUE;
	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;
	Settings.FrameTime = Settings.FrameTimeNTSC;
	Settings.DisableSampleCaching = FALSE;
	Settings.DisableMasterVolume = FALSE;
	Settings.Mouse = FALSE; // TRUE
	Settings.SuperScope = FALSE;
	Settings.MultiPlayer5 = FALSE;
	Settings.ControllerOption = SNES_MULTIPLAYER5;
	Settings.ControllerOption = 0;
	Settings.Transparency = TRUE;
	Settings.SixteenBit = TRUE;
	Settings.SupportHiRes = FALSE; // autodetected for known highres roms
	Settings.NetPlay = FALSE;
	Settings.ServerName[0] = 0;
	Settings.ThreadSound = FALSE;
	Settings.AutoSaveDelay = 30;
	Settings.ApplyCheats = TRUE;
	Settings.TurboMode = FALSE;
	Settings.TurboSkipFrames = 15;
	if (Settings.ForceNoTransparency)
		Settings.Transparency = FALSE;
	if (Settings.Transparency)
		Settings.SixteenBit = TRUE;
}

void LoadRom()
{
	if (rom_filename) {
		if (!Memory.LoadROM(rom_filename)) {
			char dir[_MAX_DIR + 1];
			char drive[_MAX_DRIVE + 1];
			char name[_MAX_FNAME + 1];
			char ext[_MAX_EXT + 1];
			char fname[_MAX_PATH + 1];

			_splitpath(rom_filename, drive, dir, name, ext);
			_makepath(fname, drive, dir, name, ext);

			strcpy(fname, S9xGetROMDirectory());
			strcat(fname, SLASH_STR);
			strcat(fname, name);

			if (ext[0]) {
				strcat(fname, ".");
				strcat(fname, ext);
			}
			_splitpath(fname, drive, dir, name, ext);
			_makepath(fname, drive, dir, name, ext);

			if (!Memory.LoadROM(fname)) {
				printf("Error opening: %s\n", rom_filename);
				OutOfMemory();
			}
		}
		Memory.LoadSRAM(S9xGetFilename(".srm"));
	} else {
		S9xExit();
	}

	if (snapshot_filename) {
#ifdef DEBUGGER
		int Flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
#endif
		if (!S9xLoadSnapshot(snapshot_filename))
			exit(1);
#ifdef DEBUGGER
		CPU.Flags |= Flags;
#endif
	}

#ifdef SNESADVANCE_SPEEDHACKS
	if (S9xFindHacks(Memory.ROMCRC32)) {
		S9xSetInfoString("Found speedhacks, applying...");
	}
#endif
}

void S9xInit()
{
	Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;

	if (!Memory.Init() || !S9xInitAPU()) {
		OutOfMemory();
	}

	S9xInitSound(Settings.SoundPlaybackRate, Settings.Stereo, Settings.SoundBufferSize);

	if (!Settings.APUEnabled)
		S9xSetSoundMute(TRUE);

#ifdef GFX_MULTI_FORMAT
	S9xSetRenderPixelFormat(RGB565);
#endif

	S9xInitDisplay(0, 0);
	if (!S9xGraphicsInit()) {
		OutOfMemory();
	}

	S9xInitInputDevices();

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

	S9xGraphicsMode();
	sprintf(String, "\"%s\" %s: %s", Memory.ROMName, TITLE, VERSION);
	S9xSetTitle(String);
}

void MainLoop()
{
	Settings.StopEmulation = FALSE;
	S9xSetSoundMute(FALSE);

	while (TRUE) {
		if (Settings.Paused) {
#ifdef DEBUGGER
			if (CPU.Flags & DEBUG_MODE_FLAG)
#endif
				S9xSetSoundMute(TRUE);

			S9xProcessEvents(FALSE);
			usleep(100000);
			S9xSetSoundMute(FALSE);
		} else {
#ifdef DEBUGGER
			if (CPU.Flags & (DEBUG_MODE_FLAG | SINGLE_STEP_FLAG))
#endif
				S9xMainLoop();
#ifdef DEBUGGER
			if (CPU.Flags & DEBUG_MODE_FLAG)
				S9xDoDebug();
#endif
			S9xProcessEvents(TRUE);
		}
	}
}

extern "C"
#undef main
    int
    main(int argc, char **argv)
{
	if (argc < S9xMinCommandLineArgs()) {
		S9xUsage();
	}

	S9xResetConfig();

	// parse commandline arguments for ROM filename
	rom_filename = S9xParseArgs(argv, argc);
	printf("Rom filename: %s\n", rom_filename);

	S9xReadConfig();
	S9xInit();

	char msg[SBUFFER];
	// Handheld Key Infos
#ifdef MIYOO
	sprintf(msg, "Press R to Show MENU");
#else
	sprintf(msg, "Press SELECT+START to Show MENU");
#endif
	S9xSetInfoString(msg);

	LoadRom();
	MainLoop();

	return (0);
}

void S9xAutoSaveSRAM() { Memory.SaveSRAM(S9xGetFilename(".srm")); }

void OutOfMemory()
{
	fprintf(stderr, "Snes9X: Memory allocation failure - not enough "
			"RAM/virtual memory available.\n S9xExiting...\n");

	Memory.Deinit();
	S9xDeinitAPU();
	S9xDeinitDisplay();

	exit(1);
}

void S9xExit()
{
	S9xSetSoundMute(TRUE);

	S9xWriteConfig();
	Memory.SaveSRAM(S9xGetFilename(".srm"));
#ifdef CHEATS
	S9xSaveCheatFile(S9xGetFilename(".cht"));
#endif

	Memory.Deinit();
	S9xDeinitAPU();
	S9xDeinitDisplay();

	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();

	exit(0);
}

void S9xInitInputDevices()
{
	keyssnes = SDL_GetKeyState(NULL);

	memset(sfc_key, 0, 256 * sizeof(uint16));

	// Controller mapping
	sfc_key[A_1] = BUTTON_A;
	sfc_key[B_1] = BUTTON_B;
	sfc_key[X_1] = BUTTON_X;
	sfc_key[Y_1] = BUTTON_Y;
	sfc_key[L_1] = BUTTON_L;
	sfc_key[R_1] = BUTTON_R;
	sfc_key[START_1] = BUTTON_START;
	sfc_key[SELECT_1] = BUTTON_SELECT;
	sfc_key[LEFT_1] = BUTTON_LEFT;
	sfc_key[RIGHT_1] = BUTTON_RIGHT;
	sfc_key[UP_1] = BUTTON_UP;
	sfc_key[DOWN_1] = BUTTON_DOWN;

#ifdef BUTTON_QUIT
	sfc_key[QUIT] = BUTTON_QUIT;
#endif

	int i = 0;
	char *envp, *j;
	envp = j = getenv("S9XKEYS");
	if (envp) {
		do {
			if ((j = strchr(envp, ',')))
				*j = 0;
			if (i == 0)
				sfc_key[QUIT] = atoi(envp);
			else if (i == 1)
				sfc_key[A_1] = atoi(envp);
			else if (i == 2)
				sfc_key[B_1] = atoi(envp);
			else if (i == 3)
				sfc_key[X_1] = atoi(envp);
			else if (i == 4)
				sfc_key[Y_1] = atoi(envp);
			else if (i == 5)
				sfc_key[L_1] = atoi(envp);
			else if (i == 6)
				sfc_key[R_1] = atoi(envp);
			else if (i == 7)
				sfc_key[START_1] = atoi(envp);
			else if (i == 8)
				sfc_key[SELECT_1] = atoi(envp);
			else if (i == 9)
				sfc_key[LEFT_1] = atoi(envp);
			else if (i == 10)
				sfc_key[RIGHT_1] = atoi(envp);
			else if (i == 11)
				sfc_key[UP_1] = atoi(envp);
			else if (i == 12)
				sfc_key[DOWN_1] = atoi(envp);
#if 0
			else if (i == 13)
				sfc_key[LU_2] = atoi(envp);
			else if (i == 14)
				sfc_key[LD_2] = atoi(envp);
			else if (i == 15)
				sfc_key[RU_2] = atoi(envp);
			else if (i == 16)
				sfc_key[RD_2] = atoi(envp);
#endif
			envp = j + 1;
			++i;
		} while (j);
	}
}

const char *GetHomeDirectory() { return (getenv("HOME")); }

const char *S9xGetSnapshotDirectory()
{
	static char filename[PATH_MAX];
	const char *snapshot;

	if (!(snapshot = getenv("SNES9X_SNAPSHOT_DIR")) && !(snapshot = getenv("SNES96_SNAPSHOT_DIR"))) {
		const char *home = GetHomeDirectory();
		strcpy(filename, home);
		strcat(filename, SLASH_STR);
		strcat(filename, ".snes96_snapshots");
		mkdir(filename, 0777);
		// chown (filename, getuid (), getgid ());
	} else
		return (snapshot);

	return (filename);
}

const char *S9xGetFilename(const char *ex)
{
	static char filename[PATH_MAX + 1];
	char drive[_MAX_DRIVE + 1];
	char dir[_MAX_DIR + 1];
	char fname[_MAX_FNAME + 1];
	char ext[_MAX_EXT + 1];

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	strcpy(filename, S9xGetSnapshotDirectory());
	strcat(filename, SLASH_STR);
	strcat(filename, fname);
	strcat(filename, ex);

	return (filename);
}

const char *S9xGetROMDirectory()
{
	const char *roms;

	if (!(roms = getenv("SNES9X_ROM_DIR")) && !(roms = getenv("SNES96_ROM_DIR")))
		return ("." SLASH_STR "roms");
	else
		return (roms);
}

const char *S9xBasename(const char *f)
{
	const char *p;
	if ((p = strrchr(f, '/')) != NULL || (p = strrchr(f, '\\')) != NULL)
		return (p + 1);

	return (f);
}

bool8 S9xOpenSnapshotFile(const char *fname, bool8 read_only, STREAM *file)
{
	char filename[PATH_MAX + 1];
	char drive[_MAX_DRIVE + 1];
	char dir[_MAX_DIR + 1];
	char ext[_MAX_EXT + 1];

	_splitpath(fname, drive, dir, filename, ext);

	if (*drive || *dir == '/' || (*dir == '.' && (*(dir + 1) == '/'))) {
		strcpy(filename, fname);
		if (!*ext)
			strcat(filename, ".s96");
	} else {
		strcpy(filename, S9xGetSnapshotDirectory());
		strcat(filename, SLASH_STR);
		strcat(filename, fname);
		if (!*ext)
			strcat(filename, ".s96");
	}

#ifdef ZLIB
	if (read_only) {
		if ((*file = OPEN_STREAM(filename, "rb")))
			return (TRUE);
	} else {
		if ((*file = OPEN_STREAM(filename, "wb"))) {
			// chown (filename, getuid (), getgid ());
			return (TRUE);
		}
	}
#else
	char command[PATH_MAX];

	if (read_only) {
		sprintf(command, "gzip -d <\"%s\"", filename);
		if (*file = popen(command, "r"))
			return (TRUE);
	} else {
		sprintf(command, "gzip --best >\"%s\"", filename);
		if (*file = popen(command, "wb"))
			return (TRUE);
	}
#endif
	return (FALSE);
}

void S9xCloseSnapshotFile(STREAM file)
{
#ifdef ZLIB
	CLOSE_STREAM(file);
#else
	pclose(file);
#endif
	sync();
}

bool8_32 S9xInitUpdate() { return (TRUE); }

bool8_32 S9xDeinitUpdate(int Width, int Height)
{
	uint32 spd = (Settings.SupportHiRes ? SNES_WIDTH / 2 : 0);
	uint32 dpd = (screen->w - SNES_WIDTH) / 2;
	uint32 dpo = (screen->w - SNES_WIDTH) / 4 + (screen->h - SNES_HEIGHT) / 2 * screen->w / 2;

	SDL_LockSurface(screen);

	if (Settings.SupportHiRes && (Width > SNES_WIDTH)) {
		// If SupportHiRes is active and HighRes Frame
		uint16 *dp16 = (uint16 *)(screen->pixels) + dpo * 2;
		uint32 *sp32 = (uint32 *)(GFX.Screen);
		for (int y = SNES_HEIGHT; y--;) {
			for (int x = SNES_WIDTH; x--;) {
				*dp16++ = *sp32++;
			}
			dp16 += dpd * 2;
		}
	} else if (Settings.SupportHiRes && Scale) {
		(*upscale_p)((uint32_t *)screen->pixels, (uint32_t *)GFX.Screen, SNES_WIDTH * 2);
	} else if (Scale) {
		(*upscale_p)((uint32_t *)screen->pixels, (uint32_t *)GFX.Screen, SNES_WIDTH);
	} else {
		uint32 *dp32 = (uint32 *)(screen->pixels) + dpo;
		uint32 *sp32 = (uint32 *)(GFX.Screen);
		for (int y = SNES_HEIGHT; y--;) {
			for (int x = SNES_WIDTH / 2; x--;) {
				*dp32++ = *sp32++;
			}
			sp32 += spd;
			dp32 += dpd;
		}
	}

	if (GFX.InfoString)
		S9xDisplayString(GFX.InfoString, (uint8 *)screen->pixels + screen->w - SNES_WIDTH, screen->pitch, 0);
	else if (Settings.DisplayFrameRate)
		S9xDisplayFrameRate((uint8 *)screen->pixels + screen->w - SNES_WIDTH, screen->pitch);

	SDL_UnlockSurface(screen);
	SDL_Flip(screen);
	return (TRUE);
}

void _makepath(char *path, const char *, const char *dir, const char *fname, const char *ext)
{
	if (dir && *dir) {
		strcpy(path, dir);
		strcat(path, "/");
	} else
		*path = 0;
	strcat(path, fname);
	if (ext && *ext) {
		strcat(path, ".");
		strcat(path, ext);
	}
}

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
	*drive = 0;

	char *slash = strrchr((char *)path, '/');
	if (!slash)
		slash = strrchr((char *)path, '\\');

	char *dot = strrchr((char *)path, '.');

	if (dot && slash && dot < slash)
		dot = NULL;

	if (!slash) {
		strcpy(dir, "");
		strcpy(fname, path);
		if (dot) {
			*(fname + (dot - path)) = 0;
			strcpy(ext, dot + 1);
		} else
			strcpy(ext, "");
	} else {
		strcpy(dir, path);
		*(dir + (slash - path)) = 0;
		strcpy(fname, slash + 1);
		if (dot) {
			*(fname + (dot - slash) - 1) = 0;
			strcpy(ext, dot + 1);
		} else
			strcpy(ext, "");
	}
}

void S9xToggleSoundChannel(int c)
{
	if (c == 8)
		so.sound_switch = 255;
	else
		so.sound_switch ^= 1 << c;
	S9xSetSoundControl(so.sound_switch);
}

inline void timespec_sub(struct timespec *diff, const struct timespec *left, const struct timespec *right)
{
	diff->tv_sec = left->tv_sec - right->tv_sec;
	diff->tv_nsec = left->tv_nsec - right->tv_nsec;
}

void S9xSyncSpeed() // called from S9xMainLoop in ../cpuexec.cpp
{
	if (!Settings.TurboMode && Settings.SkipFrames == AUTO_FRAMERATE) {
		static struct timespec next1 = {0, 0};
		struct timespec now, diff;

		while (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
			;
		if (next1.tv_sec == 0) {
			next1 = now;
			next1.tv_nsec += Settings.FrameTime * 1000;
		}

		timespec_sub(&diff, &next1, &now);

		if (diff.tv_nsec > 0) {
			if (IPPU.SkippedFrames == 0) {
				CHECK_SOUND();
				nanosleep(&diff, NULL);
				while (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
					;
			}
			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
		} else {
			if (IPPU.SkippedFrames < MaxAutoFrameSkip) {
				IPPU.SkippedFrames++;
				IPPU.RenderThisFrame = FALSE;
			} else {
				IPPU.RenderThisFrame = TRUE;
				IPPU.SkippedFrames = 0;
				next1 = now;
			}
		}
		next1.tv_nsec += Settings.FrameTime * 1000;
		if (next1.tv_nsec >= 1000000000) {
			next1.tv_sec += next1.tv_nsec / 1000000000;
			next1.tv_nsec %= 1000000000;
		}
	} else {
		if (++IPPU.FrameSkip >= (Settings.TurboMode ? Settings.TurboSkipFrames : Settings.SkipFrames)) {
			IPPU.FrameSkip = 0;
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		} else {
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
		}
	}
}

void S9xProcessEvents(bool8_32 block)
{
	SDL_Event event;

	while (block && SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			keyssnes = SDL_GetKeyState(NULL);

			// QUIT Emulator
			if ((keyssnes[sfc_key[SELECT_1]] == SDL_PRESSED) &&
			    (keyssnes[sfc_key[START_1]] == SDL_PRESSED) && (keyssnes[sfc_key[X_1]] == SDL_PRESSED)) {
				S9xExit();
			}
			// RESET ROM Playback
			else if ((keyssnes[sfc_key[SELECT_1]] == SDL_PRESSED) &&
				 (keyssnes[sfc_key[START_1]] == SDL_PRESSED) &&
				 (keyssnes[sfc_key[B_1]] == SDL_PRESSED)) {
				S9xReset();
			}
			// SAVE State
			else if ((keyssnes[sfc_key[START_1]] == SDL_PRESSED) &&
				 (keyssnes[sfc_key[R_1]] == SDL_PRESSED)) {
				// extern char snapscreen;
				char fname[256], ext[20];
				S9xSetSoundMute(TRUE);
				sprintf(ext, ".00%d", SaveSlotNum);
				strcpy(fname, S9xGetFilename(ext));
				S9xFreezeGame(fname);
				capt_screenshot();
				sprintf(ext, ".s0%d", SaveSlotNum);
				strcpy(fname, S9xGetFilename(ext));
				save_screenshot(fname);
				S9xSetSoundMute(FALSE);
			}
			// LOAD State
			else if ((keyssnes[sfc_key[START_1]] == SDL_PRESSED) &&
				 (keyssnes[sfc_key[L_1]] == SDL_PRESSED)) {
				char fname[256], ext[8];
				S9xSetSoundMute(TRUE);
				sprintf(ext, ".00%d", SaveSlotNum);
				strcpy(fname, S9xGetFilename(ext));
				S9xLoadSnapshot(fname);
				S9xSetSoundMute(FALSE);
			}
			// MAINMENU
#ifdef BUTTON_QUIT
			else if (keyssnes[sfc_key[QUIT]] == SDL_PRESSED)
#else
			else if ((keyssnes[sfc_key[SELECT_1]] == SDL_PRESSED) &&
				 (keyssnes[sfc_key[START_1]] == SDL_PRESSED))
#endif
			{
				S9xSetSoundMute(TRUE);
				menu_loop();
				S9xSetSoundMute(FALSE);
			}
			break;
		case SDL_KEYUP:
			keyssnes = SDL_GetKeyState(NULL);
			break;
		}
	}
}

uint32 S9xReadJoypad(int which1)
{
	uint32 val = 0x80000000;

	if (which1 > 4)
		return 0;

	// player1
	if (keyssnes[sfc_key[L_1]] == SDL_PRESSED)
		val |= SNES_TL_MASK;
	if (keyssnes[sfc_key[R_1]] == SDL_PRESSED)
		val |= SNES_TR_MASK;
	if (keyssnes[sfc_key[X_1]] == SDL_PRESSED)
		val |= SNES_X_MASK;
	if (keyssnes[sfc_key[Y_1]] == SDL_PRESSED)
		val |= SNES_Y_MASK;
	if (keyssnes[sfc_key[B_1]] == SDL_PRESSED)
		val |= SNES_B_MASK;
	if (keyssnes[sfc_key[A_1]] == SDL_PRESSED)
		val |= SNES_A_MASK;
	if (keyssnes[sfc_key[START_1]] == SDL_PRESSED)
		val |= SNES_START_MASK;
	if (keyssnes[sfc_key[SELECT_1]] == SDL_PRESSED)
		val |= SNES_SELECT_MASK;
	if (keyssnes[sfc_key[UP_1]] == SDL_PRESSED)
		val |= SNES_UP_MASK;
	if (keyssnes[sfc_key[DOWN_1]] == SDL_PRESSED)
		val |= SNES_DOWN_MASK;
	if (keyssnes[sfc_key[LEFT_1]] == SDL_PRESSED)
		val |= SNES_LEFT_MASK;
	if (keyssnes[sfc_key[RIGHT_1]] == SDL_PRESSED)
		val |= SNES_RIGHT_MASK;

#if 0
	// player2
	if (keyssnes[sfc_key[UP_2]] == SDL_PRESSED)
		val |= SNES_UP_MASK;
	if (keyssnes[sfc_key[DOWN_2]] == SDL_PRESSED)
		val |= SNES_DOWN_MASK;
	if (keyssnes[sfc_key[LEFT_2]] == SDL_PRESSED)
		val |= SNES_LEFT_MASK;
	if (keyssnes[sfc_key[RIGHT_2]] == SDL_PRESSED)
		val |= SNES_RIGHT_MASK;
	if (keyssnes[sfc_key[LU_2]] == SDL_PRESSED)
		val |= SNES_LEFT_MASK | SNES_UP_MASK;
	if (keyssnes[sfc_key[LD_2]] == SDL_PRESSED)
		val |= SNES_LEFT_MASK | SNES_DOWN_MASK;
	if (keyssnes[sfc_key[RU_2]] == SDL_PRESSED)
		val |= SNES_RIGHT_MASK | SNES_UP_MASK;
	if (keyssnes[sfc_key[RD_2]] == SDL_PRESSED)
		val |= SNES_RIGHT_MASK | SNES_DOWN_MASK;
#endif

	return (val);
}

#ifdef SNESADVANCE_SPEEDHACKS
int S9xFindHacks(int game_crc32)
{
	unsigned int i = 0, j;
	int _crc32;
	char c;
	char str[256];
	unsigned int size_snesadvance;
	unsigned char *snesadvance;
	FILE *f;

	sprintf(str, "%s/%s/snesadvance.dat", GetHomeDirectory(), CFG_DIRECTORY);
	f = fopen(str, "rb");
	if (!f)
		return 0;
	fseek(f, 0, SEEK_END);
	size_snesadvance = ftell(f);
	fseek(f, 0, SEEK_SET);
	snesadvance = (unsigned char *)malloc(size_snesadvance);
	fread(snesadvance, 1, size_snesadvance, f);
	fclose(f);

	for (;;) {
		// get crc32
		j = i;
		while ((i < size_snesadvance) && (snesadvance[i] != '|'))
			i++;
		if (i == size_snesadvance) {
			free(snesadvance);
			return 0;
		}
		// we have (snesadvance[i]=='|')
		// convert crc32 to int
		_crc32 = 0;
		while (j < i) {
			c = snesadvance[j];
			if ((c >= '0') && (c <= '9'))
				_crc32 = (_crc32 << 4) | (c - '0');
			else if ((c >= 'A') && (c <= 'F'))
				_crc32 = (_crc32 << 4) | (c - 'A' + 10);
			else if ((c >= 'a') && (c <= 'f'))
				_crc32 = (_crc32 << 4) | (c - 'a' + 10);
			j++;
		}
		if (game_crc32 == _crc32) {
			// int p=0;
			for (;;) {
				int adr, val;

				i++;
				j = i;
				while ((i < size_snesadvance) && (snesadvance[i] != 0x0D) && (snesadvance[i] != ',')) {
					if (snesadvance[i] == '|')
						j = i + 1;
					i++;
				}
				if (i == size_snesadvance) {
					free(snesadvance);
					return 0;
				}
				memcpy(str, &snesadvance[j], i - j);
				str[i - j] = 0;
				sscanf(str, "%X=%X", &adr, &val);
				// sprintf(str,"read : %X=%X",adr,val);
				// pgPrintAllBG(32,31-p++,0xFFFF,str);

				if ((val == 0x42) || ((val & 0xFF00) == 0x4200)) {
					// Settings.SNESAdvanceHack = true;
					if (val & 0xFF00) {
						Memory.ROM[adr] = (val >> 8) & 0xFF;
						Memory.ROM[adr + 1] = val & 0xFF;
					} else
						Memory.ROM[adr] = val;
				}

				if (snesadvance[i] == 0x0D) {
					free(snesadvance);
					return 1;
				}
			}
		}
		while ((i < size_snesadvance) && (snesadvance[i] != 0x0A))
			i++;
		if (i == size_snesadvance) {
			free(snesadvance);
			return 0;
		}
		i++; // new line
	}
}
#endif
