#include <SDL/SDL.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <strings.h>
#include "keydef.h"
#include "dingoo.h"

#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"
#include "snapshot.h"
#include "display.h"
#include "gfx.h"
#include "unistd.h"

#include "sdlmenu.h"
#include "sdlaudio.h"
#include "sdlvideo.h"

extern Uint16 sfc_key[SBUFFER];
extern bool8_32 Scale;
#ifdef BILINEAR_SCALE
extern bool8_32 Bilinear;
#endif
extern short SaveSlotNum;
extern short vol;

extern void S9xDisplayString(const char *string, uint8 *, uint32, int ypos);
void save_screenshot(char *fname);
void load_screenshot(char *fname);
void show_screenshot(void);
void capt_screenshot(void);
void menu_dispupdate(void);
void show_credits(void);

int cursor = 3;
char SaveSlotNum_old = 255;
bool8_32 highres_current = FALSE;
char snapscreen[17120] = {};

#define strfmt(str, format, args...)                                           \
	do {                                                                   \
		char tmp[SBUFFER];                                             \
		snprintf(tmp, sizeof(tmp) - 1, format, args);                  \
		strncpy(str, tmp, sizeof(tmp));                                \
	} while (0)

void sys_sleep(int us)
{
	if (us > 0)
		SDL_Delay(us / 100);
}

extern SDL_Surface *screen, *gfxscreen;

void menu_flip()
{
	SDL_Rect dst;

	dst.x = (screen->w - MENU_WIDTH) / 2;
	dst.y = (screen->h - MENU_HEIGHT) / 2;
	SDL_BlitSurface(gfxscreen, NULL, screen, &dst);
	SDL_Flip(screen);
}

void menu_init()
{
	const int color = 0x0; // black
	for (int y = 0; y <= MENU_HEIGHT; y++) {
		for (int x = 0; x < MENU_WIDTH * 2; x += 2) {
			memset(GFX.Screen + GFX.Pitch * y + x, color, 2);
		}
	}
}

void menu_dispupdate(void)
{
	const char *Rates[8] = {"off",	 "8192",  "11025", "16000",
				"22050", "32000", "44100", "48000"};
	char disptxt[MAX_MENU_ITEMS][SBUFFER] = {
#ifdef MIYOO
	    "  Snes9x4D " BUILD_VERSION " for Miyoo  ",
#elif
	    "  Snes9x4D " BUILD_VERSION " for SDL  ",
#endif
	    "",
	    "Reset Game               ",
	    "Save State               ",
	    "Load State               ",
	    "State Slot               ",
	    "Display Frame Rate       ",
	    "Transparency             ",
	    "Full Screen              ",
#ifdef BILINEAR_SCALE
	    "Bilinear Filtering       ",
#else
	    "N/A",
#endif
	    "Frameskip                ",
	    "Sound Rate               ",
	    "Stereo                   ",
	    "Credits                  ",
	    "Exit                     "};

	menu_init();

	strfmt(disptxt[5], "%s No. %d", disptxt[5], SaveSlotNum);

	if (Settings.DisplayFrameRate)
		strfmt(disptxt[6], "%s True", disptxt[6]);
	else
		strfmt(disptxt[6], "%s False", disptxt[6]);

	if (Settings.Transparency)
		strfmt(disptxt[7], "%s On", disptxt[7]);
	else
		strfmt(disptxt[7], "%s Off", disptxt[7]);

	if (Scale)
		strfmt(disptxt[8], "%s True", disptxt[8]);
	else
		strfmt(disptxt[8], "%s False", disptxt[8]);

#ifdef BILINEAR_SCALE
	if (Bilinear)
		strfmt(disptxt[9], "%s True", disptxt[9]);
	else
		strfmt(disptxt[9], "%s False", disptxt[9]);
#endif

	if (Settings.SkipFrames == AUTO_FRAMERATE)
		strfmt(disptxt[10], "%s Auto", disptxt[10]);
	else
		strfmt(disptxt[10], "%s %02d/%d", disptxt[10],
		       (int)Memory.ROMFramesPerSecond, Settings.SkipFrames);

	strfmt(disptxt[11], "%s %s", disptxt[11],
	       Rates[Settings.SoundPlaybackRate]);

	if (Settings.Stereo)
		strfmt(disptxt[12], "%s True", disptxt[12]);
	else
		strfmt(disptxt[12], "%s False", disptxt[12]);

	for (int i = 0; i < MAX_MENU_ITEMS; i++) {
		if (i == cursor)
			strfmt(disptxt[i], " >%s", disptxt[i]);
		else
			strfmt(disptxt[i], "  %s", disptxt[i]);

		S9xDisplayString(disptxt[i], GFX.Screen, GFX.Pitch,
				 i * 10 + 64);
	}

	// show screen shot for snapshot
	if (SaveSlotNum_old != SaveSlotNum) {
		char temp[SBUFFER];
		strcpy(temp, "Loading...");
		S9xDisplayString(temp, GFX.Screen + 280, GFX.Pitch,
				 210 /*204*/);
		menu_flip();
		char fname[SBUFFER], ext[8];
		sprintf(ext, ".s0%d", SaveSlotNum);
		strcpy(fname, S9xGetFilename(ext));
		load_screenshot(fname);
		SaveSlotNum_old = SaveSlotNum;
	}
	show_screenshot();
	menu_flip();
}

void menu_loop(void)
{
	bool old_stereo = Settings.Stereo;
	int old_sound_playback_rate = Settings.SoundPlaybackRate;
	bool8_32 exit_loop = false;
	char fname[SBUFFER], ext[8];
	char snapscreen_tmp[sizeof(snapscreen)];

	uint8 *keyssnes = 0;

	SaveSlotNum_old = -1;

	highres_current = Settings.SupportHiRes;

	capt_screenshot();
	memcpy(snapscreen_tmp, snapscreen, sizeof(snapscreen));

	Settings.SupportHiRes = FALSE;
	S9xDeinitDisplay();
	S9xInitDisplay(0, 0);

	menu_dispupdate();
	sys_sleep(10000);

	SDL_Event event;

	do {
		while (SDL_PollEvent(&event) == 1) {
			keyssnes = SDL_GetKeyState(NULL);

			if (keyssnes[sfc_key[UP_1]] == SDL_PRESSED)
				cursor--;
			else if (keyssnes[sfc_key[DOWN_1]] == SDL_PRESSED)
				cursor++;
			else {
				switch (cursor) {
				case 2:
					if ((keyssnes[sfc_key[A_1]] ==
					     SDL_PRESSED)) {
						S9xReset();
						exit_loop = TRUE;
					}
					break;
				case 3:
					if (keyssnes[sfc_key[A_1]] ==
					    SDL_PRESSED) {
						memcpy(snapscreen,
						       snapscreen_tmp, 16050);
						show_screenshot();
						strcpy(fname, " Saving...");
						S9xDisplayString(
						    fname, GFX.Screen + 280,
						    GFX.Pitch, 204);
						menu_flip();
						sprintf(ext, ".s0%d",
							SaveSlotNum);
						strcpy(fname,
						       S9xGetFilename(ext));
						save_screenshot(fname);
						sprintf(ext, ".00%d",
							SaveSlotNum);
						strcpy(fname,
						       S9xGetFilename(ext));
						S9xFreezeGame(fname);
						sync();
						exit_loop = TRUE;
					}
					break;
				case 4:
					if (keyssnes[sfc_key[A_1]] ==
					    SDL_PRESSED) {
						sprintf(ext, ".00%d",
							SaveSlotNum);
						strcpy(fname,
						       S9xGetFilename(ext));
						S9xLoadSnapshot(fname);
						exit_loop = TRUE;
					}
					break;
				case 5:
					if (keyssnes[sfc_key[LEFT_1]] ==
					    SDL_PRESSED)
						SaveSlotNum--;
					else if (keyssnes[sfc_key[RIGHT_1]] ==
						 SDL_PRESSED)
						SaveSlotNum++;

					if (SaveSlotNum > MAX_SAVE_STATE_SLOTS)
						SaveSlotNum = 0;
					else if (SaveSlotNum < 0)
						SaveSlotNum =
						    MAX_SAVE_STATE_SLOTS;
					break;
				case 6:
					if (keyssnes[sfc_key[LEFT_1]] ==
						SDL_PRESSED ||
					    keyssnes[sfc_key[RIGHT_1]] ==
						SDL_PRESSED)
						Settings.DisplayFrameRate =
						    !Settings.DisplayFrameRate;
					break;
				case 7:
					if (keyssnes[sfc_key[LEFT_1]] ==
						SDL_PRESSED ||
					    keyssnes[sfc_key[RIGHT_1]] ==
						SDL_PRESSED)
						Settings.Transparency =
						    !Settings.Transparency;
					break;
				case 8:
					if (keyssnes[sfc_key[LEFT_1]] ==
						SDL_PRESSED ||
					    keyssnes[sfc_key[RIGHT_1]] ==
						SDL_PRESSED)
						Scale = !Scale;
					break;
				case 9:
#ifdef BILINEAR_SCALE
					if (keyssnes[sfc_key[LEFT_1]] ==
						SDL_PRESSED ||
					    keyssnes[sfc_key[RIGHT_1]] ==
						SDL_PRESSED)
						Bilinear = !Bilinear;
#endif
					break;
				case 10:
					if (Settings.SkipFrames ==
					    AUTO_FRAMERATE)
						Settings.SkipFrames = 10;

					if (keyssnes[sfc_key[LEFT_1]] ==
					    SDL_PRESSED)
						Settings.SkipFrames--;
					else if (keyssnes[sfc_key[RIGHT_1]] ==
						 SDL_PRESSED)
						Settings.SkipFrames++;

					if (Settings.SkipFrames >= 10)
						Settings.SkipFrames =
						    AUTO_FRAMERATE;
					else if (Settings.SkipFrames <= 1)
						Settings.SkipFrames = 1;
					break;
				case 11:
					if (keyssnes[sfc_key[LEFT_1]] ==
					    SDL_PRESSED) {
						Settings.SoundPlaybackRate =
						    (Settings
							 .SoundPlaybackRate -
						     1) &
						    7;
					} else if (keyssnes[sfc_key[RIGHT_1]] ==
						   SDL_PRESSED) {
						Settings.SoundPlaybackRate =
						    (Settings
							 .SoundPlaybackRate +
						     1) &
						    7;
					}
					break;
				case 12:
					if (keyssnes[sfc_key[LEFT_1]] ==
						SDL_PRESSED ||
					    keyssnes[sfc_key[RIGHT_1]] ==
						SDL_PRESSED)
						Settings.Stereo =
						    !Settings.Stereo;
					break;
				case 13:
					if (keyssnes[sfc_key[A_1]] ==
					    SDL_PRESSED)
						show_credits();
					break;
				case 14:
					if (keyssnes[sfc_key[A_1]] ==
					    SDL_PRESSED)
						S9xExit();
					break;
				}
			}

			if (cursor <= 1)
				cursor = MAX_MENU_ITEMS - 1;
			else if (cursor >= MAX_MENU_ITEMS)
				cursor = 2;

			menu_dispupdate();
			sys_sleep(1000);

			break;
		}
	} while (exit_loop != TRUE && keyssnes[sfc_key[B_1]] != SDL_PRESSED);

	Settings.SupportHiRes = highres_current;
	S9xDeinitDisplay();
	S9xInitDisplay(0, 0);
	if (old_sound_playback_rate != Settings.SoundPlaybackRate ||
	    old_stereo != Settings.Stereo)
		S9xReinitSound();
}

void save_screenshot(char *fname)
{
	FILE *fs = fopen(fname, "wb");
	if (fs == NULL) {
		S9xSetInfoString("Failed to save screenshot");
		return;
	}

	fwrite(snapscreen, sizeof(snapscreen), 1, fs);
	fflush(fs);
	fclose(fs);
}

void load_screenshot(char *fname)
{
	FILE *fs = fopen(fname, "rb");
	if (fs == NULL) {
		memset(&snapscreen, 0x0, sizeof(snapscreen));
		return;
	}
	fread(snapscreen, sizeof(snapscreen), 1, fs);
	fclose(fs);
}

void capt_screenshot() // 107px*80px
{
	bool8_32 Scale_disp = Scale;
	int s = 0;
	int yoffset = 0;
	struct InternalPPU *ippu = &IPPU;

	memset(&snapscreen, 0x0, sizeof(snapscreen));

	if (ippu->RenderedScreenHeight == MENU_HEIGHT)
		yoffset = 8;

	if (highres_current == TRUE)
		Scale_disp = FALSE;

	for (int y = yoffset; y < SURFACE_HEIGHT - yoffset; y += 3) // 240/3=80
	{
		s += 22 * (Scale_disp != TRUE);
		for (int x = 0;
		     x < SURFACE_WIDTH * 2 - 128 * (Scale_disp != TRUE);
		     x += 3 * 2) // 640/6=107
		{
			uint8 *d = GFX.Screen + y * GFX.Pitch + x;
			snapscreen[s++] = *d++;
			snapscreen[s++] = *d++;
		}
		s += 20 * (Scale_disp != TRUE);
	}
}

void show_screenshot()
{
	int s = 0;
	const int start_y = SURFACE_HEIGHT - 88;
	const int start_x = SURFACE_WIDTH - 72;

	for (int y = start_y; y < start_y + 80; y++) {
		for (int x = start_x; x < start_x + 107 * 2; x += 2) {
			uint8 *d = GFX.Screen + y * GFX.Pitch + x;
			*d++ = snapscreen[s++];
			*d++ = snapscreen[s++];
		}
	}
}

void show_credits()
{
	uint8 *keyssnes = 0;
	int line = 0, ypix = 0;
	char disptxt[100][SBUFFER] = {
	    "",
	    "",
	    "",
	    "",
	    "",
	    "                                     ",
	    " Thank you using this Emulator!      ",
	    "                                     ",
	    "",
	    "",
	    "",
	    "",
	    "",
	    "",
	    "",
	    " by SiENcE",
	    " crankgaming.blogspot.com",
	    "",
	    " regards to joyrider & g17",
	    "",
	    "",
	    "",
	    "",
	    "",
	    "",
	    "",
	    " ported by m45t3r",
	    "",
	    " with optimizations from drowsnug95 and snes9x2002",
	};

	do {
		SDL_Event event;
		SDL_PollEvent(&event);

		keyssnes = SDL_GetKeyState(NULL);

		menu_init();

		for (int i = 0; i <= 16; i++) {
			int j = i + line;
			if (j >= 20)
				j -= 20;
			S9xDisplayString(disptxt[j], GFX.Screen, GFX.Pitch,
					 i * 10 + 80 - ypix);
		}

		ypix += 2;
		if (ypix == 12) {
			line++;
			ypix = 0;
		}
		if (line == 20)
			line = 0;
		menu_flip();
		sys_sleep(3000);
	} while (keyssnes[sfc_key[B_1]] != SDL_PRESSED);

	return;
}
