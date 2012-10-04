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

#include "sdlaudio.h"

#ifdef WIN32
static void sync() { }
#endif

extern Uint16 sfc_key[256];
extern bool8_32 Scale;
extern char SaveSlotNum;
extern short vol;

extern void S9xDisplayString (const char *string, uint8 *, uint32, int ypos);
void save_screenshot(char *fname);
void load_screenshot(char *fname);
void show_screenshot(void);
void capt_screenshot(void);
void menu_dispupdate(void);
void ShowCredit(void);

int cursor = 3;
int loadcursor = 0;
int romcount_maxrows = 16;

char SaveSlotNum_old=255;
bool8_32 highres_current = FALSE;
char snapscreen[17120]={};

char temp[256];
char disptxt[20][256];

void sys_sleep(int us)
{
	if(us>0)
		SDL_Delay(us/100);
}

extern SDL_Surface *screen, *gfxscreen;

void menu_flip()
{
	SDL_Rect dst;

	dst.x = (screen->w - 256) / 2;
	dst.y = (screen->h - 224) / 2;
	SDL_BlitSurface(gfxscreen, NULL, screen, &dst);
	SDL_Flip(screen);
}

#ifndef DINGOO
//------------------------------------------------------------------------------------------
struct dirent **namelist;

int isFile(const struct dirent *nombre) {
 int isFile = 0;
 char *extension = rindex( (char*) nombre->d_name, '.');
 if (strcmp(extension, ".sfc") == 0 ||
 	 strcmp(extension, ".smc") == 0 ||
 	 strcmp(extension, ".zip" ) == 0 )
 {
  isFile = 1;
 }

 return isFile;
}

int FileDir(char *dir, const char *ext)
{
	int n;

	//printf ("Try to create ./roms directory..");
#ifdef WIN32
	mkdir (dir);
#else
	mkdir (dir, 0777);
	chown (dir, getuid (), getgid ());
#endif

	n = scandir (dir, &namelist, isFile, alphasort);
	if (n >= 0)
	{
//		int cnt;
//		for (cnt = 0; cnt < n; ++cnt)
//			puts (namelist[cnt]->d_name);
	}
	else
	{
		perror ("Couldn't open ./roms directory..try to create this directory");
	#ifdef WIN32
		mkdir (dir);
	#else
		mkdir (dir, 0777);
		chown (dir, getuid (), getgid ());
	#endif
		n = scandir (dir, &namelist, isFile, alphasort);
	}
		
	return n;
}

void loadmenu_dispupdate(int romcount)
{
	//draw blue screen
	for(int y=12;y<=212;y++){
		for(int x=10;x<246*2;x+=2){
			memset(GFX.Screen + GFX.Pitch*y+x,0x11,2);
		}	
	}

#if CAANOO
	strcpy(disptxt[0],"  Snes9x4C v20101010");
#elif CYGWIN32
	strcpy(disptxt[0],"  Snes9x4W v20101010");
#else
	strcpy(disptxt[0],"  Snes9x4D v20101010 for OpenDingux");
#endif

	//copy roms filenames to disp[] cache
	for(int i=0;i<=romcount_maxrows;i++)
	{
		if (loadcursor>romcount_maxrows)
		{
			if((i+(loadcursor-romcount_maxrows))==loadcursor)
				sprintf(temp," >%s",namelist[ i+(loadcursor-romcount_maxrows) ]->d_name);
			else
				sprintf(temp,"  %s",namelist[ i+(loadcursor-romcount_maxrows) ]->d_name);

			strncpy(disptxt[i+2],temp,34);
			disptxt[i+2][34]='\0';
		}
		else
		if (i<romcount)
		{
			if(i==loadcursor)
				sprintf(temp," >%s",namelist[i]->d_name);
			else
				sprintf(temp,"  %s",namelist[i]->d_name);

			strncpy(disptxt[i+2],temp,34);
			disptxt[i+2][34]='\0';
		}
	}

	//draw 20 lines on screen
	for(int i=0;i<19;i++)
	{
		S9xDisplayString (disptxt[i], GFX.Screen, GFX.Pitch, i*10+64);
	}

	//update screen
	menu_flip();
}

char* menu_romselector()
{
	char *rom_filename = NULL;
	int romcount = 0;

	bool8_32 exit_loop = false;

#ifdef CAANOO
	SDL_Joystick* keyssnes = 0;
#else
	uint8 *keyssnes = 0;
#endif

	//Read ROM-Directory
	romcount = FileDir("./roms", "sfc,smc");

	highres_current=Settings.SupportHiRes;

	Settings.SupportHiRes=FALSE;
	S9xDeinitDisplay();
	S9xInitDisplay(0, 0);
	
	loadmenu_dispupdate(romcount);
	sys_sleep(10000);

	SDL_Event event;

	do
	{
		loadmenu_dispupdate(romcount);
		sys_sleep(100);

		while(SDL_PollEvent(&event)==1)
		{
#ifdef CAANOO
			// CAANOO -------------------------------------------------------------
			keyssnes = SDL_JoystickOpen(0);
			if ( (SDL_JoystickGetAxis(keyssnes, 1) < -16384) || SDL_JoystickGetButton(keyssnes, sfc_key[START_1]) )
				loadcursor--;
			else if( (SDL_JoystickGetAxis(keyssnes, 1) > 16384) || SDL_JoystickGetButton(keyssnes, sfc_key[SELECT_1]) )
				loadcursor++;
//			else if ( SDL_JoystickGetButton(keyssnes, sfc_key[L_1]) )
//				loadcursor=loadcursor-10;
//			else if ( SDL_JoystickGetButton(keyssnes, sfc_key[R_1]) )
//				loadcursor=loadcursor+10;
			else if ( SDL_JoystickGetButton(keyssnes, sfc_key[QUIT]) )
				S9xExit();
			else if( SDL_JoystickGetButton(keyssnes, sfc_key[A_1]) ||
					(SDL_JoystickGetAxis(keyssnes, 0) < -16384) ||
					(SDL_JoystickGetAxis(keyssnes, 0) > 16384)
					)
			{
					switch(loadcursor)
					{
						default:
							if ( SDL_JoystickGetButton(keyssnes, sfc_key[A_1]) )
							{
								if ((loadcursor>=0) && (loadcursor<(romcount)))
								{
									rom_filename=namelist[loadcursor]->d_name;
									exit_loop = TRUE;
								}
							}
							break;
					}
			}
#else
			// DINGOO & WIN32 -----------------------------------------------------
			keyssnes = SDL_GetKeyState(NULL);
			switch(event.type)
			{
				case SDL_KEYDOWN:
					keyssnes = SDL_GetKeyState(NULL);

					//UP
					if(keyssnes[sfc_key[UP_1]] == SDL_PRESSED)
						loadcursor--;
					//DOWN
					else if(keyssnes[sfc_key[DOWN_1]] == SDL_PRESSED)
						loadcursor++;
//					//LS
//					else if(keyssnes[sfc_key[L_1]] == SDL_PRESSED)
//						loadcursor=loadcursor-10;
//					//RS
//					else if(keyssnes[sfc_key[R_1]] == SDL_PRESSED)
//						loadcursor=loadcursor+10;
					//QUIT Emulator : press ESCAPE KEY
					else if (keyssnes[sfc_key[SELECT_1]] == SDL_PRESSED)
						S9xExit();
					else if( (keyssnes[sfc_key[B_1]] == SDL_PRESSED) )
					{
						switch(loadcursor)
						{
							default:
								if ((keyssnes[sfc_key[B_1]] == SDL_PRESSED))
								{
									if ((loadcursor>=0) && (loadcursor<(romcount)))
									{
										rom_filename=namelist[loadcursor]->d_name;
										exit_loop = TRUE;
									}
								}
								break;
						}
					}
					break;
			}
#endif

			if(loadcursor==-1)
			{
				loadcursor=romcount-1;
			}
			else
			if(loadcursor==romcount)
			{
				loadcursor=0;
			}

			break;
		}
	}
#ifdef CAANOO
	while( exit_loop!=TRUE && SDL_JoystickGetButton(keyssnes, sfc_key[QUIT])!=TRUE );
#elif CYGWIN32
	while( exit_loop!=TRUE && keyssnes[sfc_key[SELECT_1]] != SDL_PRESSED );
#else
	while( exit_loop!=TRUE && keyssnes[sfc_key[B_1]] != SDL_PRESSED );
#endif

	// TODO:
	///free(). 	namelist

	Settings.SupportHiRes=highres_current;
	S9xDeinitDisplay();
	S9xInitDisplay(0, 0);

	return (rom_filename);
}

//------------------------------------------------------------------------------------------
#endif // DINGOO

void menu_dispupdate(void)
{
	static char *Rates[8] = { "off", "8192", "11025", "16000", "22050", "32000", "44100", "48000" };
//	char temp[256];
//	char disptxt[20][256];

	//memset(GFX.Screen + 320*12*2,0x11,320*200*2);
	for(int y=12;y<=212;y++){
		for(int x=10;x<246*2;x+=2){
			memset(GFX.Screen + GFX.Pitch*y+x,0x11,2);
		}	
	}
#if CAANOO
	strcpy(disptxt[0],"Snes9x4C v20101010");
#elif CYGWIN32
	strcpy(disptxt[0],"Snes9x4W v20101010");
#else
	strcpy(disptxt[0],"Snes9x4D v20101010 for OpenDingux");
#endif
	strcpy(disptxt[1],"");
	strcpy(disptxt[2],"Reset Game           ");
	strcpy(disptxt[3],"Save State           ");
	strcpy(disptxt[4],"Load State           ");
	strcpy(disptxt[5],"State Slot              No.");
	strcpy(disptxt[6],"Display Frame Rate     ");
	strcpy(disptxt[7],"Transparency           ");
	strcpy(disptxt[8],"Full Screen         ");
	strcpy(disptxt[9],"Frameskip              ");
	strcpy(disptxt[10],"Sound Rate           ");
	strcpy(disptxt[11],"Credit              ");
	strcpy(disptxt[12],"Exit");

	sprintf(temp,"%s%d",disptxt[5],SaveSlotNum);
	strcpy(disptxt[5],temp);

	if(Settings.DisplayFrameRate)
		sprintf(temp,"%s True",disptxt[6]);
	else
		sprintf(temp,"%sFalse",disptxt[6]);
	strcpy(disptxt[6],temp);
	
	if(Settings.Transparency)
		sprintf(temp,"%s   On",disptxt[7]);
	else
		sprintf(temp,"%s  Off",disptxt[7]);
	strcpy(disptxt[7],temp);

	if(Scale)
		sprintf(temp,"%s    True",disptxt[8]);
	else
		sprintf(temp,"%s   False",disptxt[8]);
	strcpy(disptxt[8],temp);

	if (Settings.SkipFrames == AUTO_FRAMERATE)
	{
		sprintf(temp,"%s Auto",disptxt[9]);
		strcpy(disptxt[9],temp);
	}
	else
	{
		sprintf(temp,"%s %02d/%d",disptxt[9],(int) Memory.ROMFramesPerSecond, Settings.SkipFrames);
		strcpy(disptxt[9],temp);
	}

	sprintf(temp,"%s  %s",disptxt[10], Rates[Settings.SoundPlaybackRate]);
	strcpy(disptxt[10],temp);

#ifdef DINGOO
	for(int i=0;i<=14;i++)
#else
	for(int i=0;i<=12;i++)
#endif
	{
		if(i==cursor)
			sprintf(temp," >%s",disptxt[i]);
		else
			sprintf(temp,"  %s",disptxt[i]);
		strcpy(disptxt[i],temp);

		S9xDisplayString (disptxt[i], GFX.Screen, GFX.Pitch ,i*10+64);
	}

	//show screen shot for snapshot
	if(SaveSlotNum_old != SaveSlotNum)
	{
		strcpy(temp,"Loading...");
		S9xDisplayString (temp, GFX.Screen + 280, GFX.Pitch, 210/*204*/);
		menu_flip();
		char fname[256], ext[8];
		sprintf(ext, ".s0%d", SaveSlotNum);
		strcpy(fname, S9xGetFilename (ext));
		load_screenshot(fname);
		SaveSlotNum_old = SaveSlotNum;
	}
	show_screenshot();
	menu_flip();
}

void menu_loop(void)
{
	int old_frame_rate = Settings.SoundPlaybackRate;
	bool8_32 exit_loop = false;
	char fname[256], ext[8];
	char snapscreen_tmp[17120];

#ifdef CAANOO
	SDL_Joystick* keyssnes = 0;
#else
	uint8 *keyssnes = 0;
#endif

	SaveSlotNum_old = -1;

	highres_current=Settings.SupportHiRes;

	capt_screenshot();
	memcpy(snapscreen_tmp,snapscreen,17120);

	Settings.SupportHiRes=FALSE;
	S9xDeinitDisplay();
	S9xInitDisplay(0, 0);

	menu_dispupdate();
	sys_sleep(10000);

	SDL_Event event;

	do
	{
		while(SDL_PollEvent(&event)==1)
		{
#ifdef CAANOO
				keyssnes = SDL_JoystickOpen(0);
				// CAANOO -------------------------------------------------------------
				if ( (SDL_JoystickGetAxis(keyssnes, 1) < -16384) || SDL_JoystickGetButton(keyssnes, sfc_key[START_1]) )
					cursor--;
				else if( (SDL_JoystickGetAxis(keyssnes, 1) > 16384) || SDL_JoystickGetButton(keyssnes, sfc_key[SELECT_1]) )
					cursor++;
				else if( SDL_JoystickGetButton(keyssnes, sfc_key[A_1]) ||
						(SDL_JoystickGetAxis(keyssnes, 0) < -16384) ||
						(SDL_JoystickGetAxis(keyssnes, 0) > 16384)
						)
				{
					switch(cursor)
					{
						case 2:
							if ( SDL_JoystickGetButton(keyssnes, sfc_key[A_1]) )
							{
								S9xReset();
								exit_loop = TRUE;
							}
						break;
						case 3:
							if ( SDL_JoystickGetButton(keyssnes, sfc_key[A_1]) )
							{
								memcpy(snapscreen,snapscreen_tmp,16050);
								show_screenshot();
								strcpy(fname," Saving...");
								S9xDisplayString (fname, GFX.Screen +280, GFX.Pitch, 204);
								menu_flip();
								sprintf(ext, ".s0%d", SaveSlotNum);
								strcpy(fname, S9xGetFilename (ext));
								save_screenshot(fname);
								sprintf(ext, ".00%d", SaveSlotNum);
								strcpy(fname, S9xGetFilename (ext));
								S9xFreezeGame (fname);
								sync();
								exit_loop = TRUE;
							}
						break;
						case 4:
							if ( SDL_JoystickGetButton(keyssnes, sfc_key[A_1]) )
							{
								sprintf(ext, ".00%d", SaveSlotNum);
								strcpy(fname, S9xGetFilename (ext));
								S9xLoadSnapshot (fname);
								exit_loop = TRUE;
							}
						break;
						case 5:
							if ( SDL_JoystickGetAxis(keyssnes, 0) < -16384 )
								SaveSlotNum--;
							else if (SDL_JoystickGetAxis(keyssnes, 0) > 16384)
								SaveSlotNum++;

							if(SaveSlotNum>=3)
								SaveSlotNum=3;			//0
							else if(SaveSlotNum<=0)
								SaveSlotNum=0;			//3
						break;
						case 6:
							Settings.DisplayFrameRate = !Settings.DisplayFrameRate;
						break;
						case 7:
							Settings.Transparency = !Settings.Transparency;
						break;
						case 8:
							Scale = !Scale;
						break;
						case 9:
							if (Settings.SkipFrames == AUTO_FRAMERATE)
								Settings.SkipFrames = 10;
	
							if ( SDL_JoystickGetAxis(keyssnes, 0) < -16384 )
								Settings.SkipFrames--;
							else if (SDL_JoystickGetAxis(keyssnes, 0) > 16384)
								Settings.SkipFrames++;
	
							if(Settings.SkipFrames>=10)
								Settings.SkipFrames = AUTO_FRAMERATE;
							else if (Settings.SkipFrames<=1)
								Settings.SkipFrames = 1;
						break;
						case 10:
							if ( SDL_JoystickGetAxis(keyssnes, 0) < -16384 )
								vol -= 10;
							else if (SDL_JoystickGetAxis(keyssnes, 0) > 16384)
								vol += 10;

							if(vol>=100)
							{
								vol = 100;
							}
							else if (vol <=0)
							{
								vol = 0;
							}
						break;
						case 11:
							if ( SDL_JoystickGetButton(keyssnes, sfc_key[A_1]) )
								ShowCredit();
						break;
						case 12:
							if ( SDL_JoystickGetButton(keyssnes, sfc_key[A_1]) )
								S9xExit();
						break;
					}
				}
#else
				// DINGOO & WIN32 -----------------------------------------------------
				keyssnes = SDL_GetKeyState(NULL);

				if(keyssnes[sfc_key[UP_1]] == SDL_PRESSED)
					cursor--;
				else if(keyssnes[sfc_key[DOWN_1]] == SDL_PRESSED)
					cursor++;
				else if( (keyssnes[sfc_key[A_1]] == SDL_PRESSED) ||
						 (keyssnes[sfc_key[LEFT_1]] == SDL_PRESSED) ||
						 (keyssnes[sfc_key[RIGHT_1]] == SDL_PRESSED) )
				{
					switch(cursor)
					{
						case 2:
							if ((keyssnes[sfc_key[A_1]] == SDL_PRESSED))
							{
								S9xReset();
								exit_loop = TRUE;
							}
						break;
						case 3:
							if (keyssnes[sfc_key[A_1]] == SDL_PRESSED)
							{
								memcpy(snapscreen,snapscreen_tmp,16050);
								show_screenshot();
								strcpy(fname," Saving...");
								S9xDisplayString (fname, GFX.Screen +280, GFX.Pitch, 204);
								menu_flip();
								sprintf(ext, ".s0%d", SaveSlotNum);
								strcpy(fname, S9xGetFilename (ext));
								save_screenshot(fname);
								sprintf(ext, ".00%d", SaveSlotNum);
								strcpy(fname, S9xGetFilename (ext));
								S9xFreezeGame (fname);
								sync();
								exit_loop = TRUE;
							}
						break;
						case 4:
							if (keyssnes[sfc_key[A_1]] == SDL_PRESSED)
							{
								sprintf(ext, ".00%d", SaveSlotNum);
								strcpy(fname, S9xGetFilename (ext));
								S9xLoadSnapshot (fname);
								exit_loop = TRUE;
							}
						break;
						case 5:
							if (keyssnes[sfc_key[LEFT_1]] == SDL_PRESSED)
								SaveSlotNum--;
							else
							if (keyssnes[sfc_key[RIGHT_1]] == SDL_PRESSED)
								SaveSlotNum++;

							if(SaveSlotNum>=3)
								SaveSlotNum=3;		//3
							else if(SaveSlotNum<=0)
								SaveSlotNum=0;		//3
						break;
						case 6:
							Settings.DisplayFrameRate = !Settings.DisplayFrameRate;
						break;
						case 7:
							Settings.Transparency = !Settings.Transparency;
						break;
						case 8:
							Scale = !Scale;
						break;
						case 9:
							if (Settings.SkipFrames == AUTO_FRAMERATE)
								Settings.SkipFrames = 10;
	
							if (keyssnes[sfc_key[LEFT_1]] == SDL_PRESSED)
								Settings.SkipFrames--;
							else
								Settings.SkipFrames++;
	
							if(Settings.SkipFrames>=10)
								Settings.SkipFrames = AUTO_FRAMERATE;
							else if (Settings.SkipFrames<=1)
								Settings.SkipFrames = 1;
						break;
						case 10:
							if (keyssnes[sfc_key[LEFT_1]] == SDL_PRESSED) {
								Settings.SoundPlaybackRate = (Settings.SoundPlaybackRate - 1) & 7;
							} else if (keyssnes[sfc_key[RIGHT_1]] == SDL_PRESSED) {
								Settings.SoundPlaybackRate = (Settings.SoundPlaybackRate + 1) & 7;
							}
						break;
						case 11:
							if (keyssnes[sfc_key[A_1]] == SDL_PRESSED)
								ShowCredit();
						break;
						case 12:
							if (keyssnes[sfc_key[A_1]] == SDL_PRESSED)
								S9xExit();
						break;
					}
				}
#endif

				if(cursor==1)
					cursor=12;	//11
				else if(cursor==13)	//12
					cursor=2;
				
				menu_dispupdate();
				sys_sleep(1000);

				break;
		}
	}
#ifdef CAANOO
	while( exit_loop!=TRUE && SDL_JoystickGetButton(keyssnes, sfc_key[QUIT])!=TRUE );
#else
	while( exit_loop!=TRUE && keyssnes[sfc_key[B_1]] != SDL_PRESSED );
#endif

	Settings.SupportHiRes=highres_current;
	S9xDeinitDisplay();
	S9xInitDisplay(0, 0);
	if(old_frame_rate != Settings.SoundPlaybackRate) S9xReinitSound(Settings.SoundPlaybackRate);
}

void save_screenshot(char *fname){
	FILE  *fs = fopen (fname,"wb");
	if(fs==NULL)
		return;
	
	fwrite(snapscreen,17120,1,fs);
	fclose (fs);
	sync();
}

void load_screenshot(char *fname)
{
	FILE *fs = fopen (fname,"rb");
	if(fs==NULL){
		for(int i=0;i<17120;i++){
			snapscreen[i] = 0;
		}
		return;
	}
	fread(snapscreen,17120,1,fs);
	fclose(fs);
}

void capt_screenshot() //107px*80px
{
	bool8_32 Scale_disp=Scale;
	int s = 0;
	int yoffset = 0;
	struct InternalPPU *ippu = &IPPU;

	for(int i=0;i<17120;i++)
	{
		snapscreen[i] = 0x00;
	}

	if(ippu->RenderedScreenHeight == 224)
		yoffset = 8;

	if (highres_current==TRUE)
	{
		//working but in highres mode
		for(int y=yoffset;y<240-yoffset;y+=3) //80,1 //240,3
		{
			s += 22 * 1 /*(Scale_disp!=TRUE)*/;
			for(int x = 0; x < 640-128*1/*(Scale_disp!=TRUE)*/;x+=6) //107,1 //214,2 //428,4 +42+42
			{
				uint8 *d = GFX.Screen + y*GFX.Pitch + x; //1024
				snapscreen[s++] = *d++;
				snapscreen[s++] = *d++;
			}
			s+=20*1/*(Scale_disp!=TRUE)*/;
		}
	}
	else
	{
		//original
		for(int y=yoffset;y<240-yoffset;y+=3) // 240/3=80
		{
			s+=22*(Scale_disp!=TRUE);
			for(int x=0 ;x<640-128*(Scale_disp!=TRUE);x+=3*2) // 640/6=107
			{
				uint8 *d = GFX.Screen + y*GFX.Pitch + x;
				snapscreen[s++] = *d++;
				snapscreen[s++] = *d++;
			}
			s+=20*(Scale_disp!=TRUE);
		}
	}
}

void show_screenshot()
{
	int s=0;
//	for(int y=126;y<126+80;y++){
	for(int y=132;y<130+80;y++){
		for(int x=248; x<248+107*2; x+=2){
			uint8 *d = GFX.Screen + y*GFX.Pitch + x;
			*d++ = snapscreen[s++];
			*d++ = snapscreen[s++];
		}
	}
}

void ShowCredit()
{
#ifdef CAANOO
	SDL_Joystick* keyssnes = 0;
#else
	uint8 *keyssnes = 0;
#endif
	int line=0,ypix=0;
	char disptxt[100][256]={
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
	};

	do
	{
		SDL_Event event;
		SDL_PollEvent(&event);

#ifdef CAANOO
		keyssnes = SDL_JoystickOpen(0);
#else
		keyssnes = SDL_GetKeyState(NULL);
#endif

		for(int y=12; y<=212; y++){
			for(int x=10; x<246*2; x+=2){
				memset(GFX.Screen + GFX.Pitch*y+x,0x11,2);
			}	
		}
		
		for(int i=0;i<=16;i++){
			int j=i+line;
			if(j>=20) j-=20;
			S9xDisplayString (disptxt[j], GFX.Screen, GFX.Pitch, i*10+80-ypix);
		}
		
		ypix+=2;
		if(ypix==12) {
			line++;
			ypix=0;
		}
		if(line == 20) line = 0;
		menu_flip();
		sys_sleep(3000);
	}
#ifdef CAANOO
	while( SDL_JoystickGetButton(keyssnes, sfc_key[B_1])!=TRUE );
#else
	while(keyssnes[sfc_key[B_1]] != SDL_PRESSED);
#endif

	return;
}

