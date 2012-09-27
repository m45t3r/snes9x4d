#include <SDL/SDL.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
//#include <sys/mman.h>
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

#ifdef WIN32
static void sync() { }
#endif

#ifdef PANDORA
	#include "pandora_scaling/blitscale.h"
	extern blit_scaler_option_t blit_scalers[];
	extern blit_scaler_e g_scale;
#endif

#ifdef DINGOO
	int batt_level(void);
	int chk_hold(void);
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
bool8_32 Scale_org=Scale;
bool8_32 highres_current = false;
char snapscreen[17120]={};

char temp[256];
char disptxt[20][256];

void sys_sleep(int us)
{
	if(us>0)
		SDL_Delay(us/100);
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
	mkdir (dir, 0777);
	chown (dir, getuid (), getgid ());

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

		mkdir (dir, 0777);
		chown (dir, getuid (), getgid ());
	
		n = scandir (dir, &namelist, isFile, alphasort);
	}
		
	return n;
}

void loadmenu_dispupdate(int romcount)
{
	//draw blue screen
	for(int y=12;y<=212;y++){
		for(int x=10;x<246*2;x+=2){
			memset(GFX.Screen + 320*y*2+x,0x11,2);
		}	
	}

#if CAANOO
	strcpy(disptxt[0],"  Snes9x4C v20101010");
#elif PANDORA
	strcpy(disptxt[0],"  Snes9x4P v20101111");
#elif CYGWIN32
	strcpy(disptxt[0],"  Snes9x4W v20101010");
#else
	strcpy(disptxt[0],"  Snes9x4D v20101010");
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
		S9xDisplayString (disptxt[i], GFX.Screen, 640,i*10+64);
	}

	//update screen
	S9xDeinitUpdate (320, 240);
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

	Scale_org = Scale;
	highres_current=Settings.SupportHiRes;

	Scale = false;
	Settings.SupportHiRes=FALSE;
	S9xDeinitDisplay();
	S9xInitDisplay(0, 0);
	
	loadmenu_dispupdate(romcount);
	sys_sleep(100000);

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
			//PANDORA & DINGOO & WIN32 -----------------------------------------------------
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

	Scale = Scale_org;
	Settings.SupportHiRes=highres_current;
	S9xDeinitDisplay();
	S9xInitDisplay(0, 0);

	return (rom_filename);
}

//------------------------------------------------------------------------------------------
#endif // DINGOO

void menu_dispupdate(void)
{
//	char temp[256];
//	char disptxt[20][256];

	//memset(GFX.Screen + 320*12*2,0x11,320*200*2);
	for(int y=12;y<=212;y++){
		for(int x=10;x<246*2;x+=2){
			memset(GFX.Screen + 320*y*2+x,0x11,2);
		}	
	}
#if CAANOO
	strcpy(disptxt[0],"Snes9x4C v20101010");
#elif PANDORA
	strcpy(disptxt[0],"Snes9x4P v20101106");
#elif CYGWIN32
	strcpy(disptxt[0],"Snes9x4W v20101010");
#else
	strcpy(disptxt[0],"Snes9x4D v20101010");
#endif
	strcpy(disptxt[1],"");
	strcpy(disptxt[2],"Reset Game           ");
	strcpy(disptxt[3],"Save State           ");
	strcpy(disptxt[4],"Load State           ");
	strcpy(disptxt[5],"State Slot              No.");
	strcpy(disptxt[6],"Display Frame Rate     ");
	strcpy(disptxt[7],"Transparency           ");
#ifdef PANDORA
	strcpy(disptxt[8],"Display mode        ");
#else
	strcpy(disptxt[8],"Full Screen         ");
#endif
	strcpy(disptxt[9],"Frameskip              ");
	strcpy(disptxt[10],"Sound Volume           ");
	strcpy(disptxt[11],"Credit              ");
	strcpy(disptxt[12],"Exit");
#ifdef DINGOO
	strcpy(disptxt[13],"--------------");
	strcpy(disptxt[14],"BATT:");
//	strcpy(disptxt[15],"     ");
//	strcpy(disptxt[16],"MHZ :");
//	strcpy(disptxt[17],"BACKLIGHT:");
#endif

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

#ifdef PANDORA
	{
	  sprintf ( temp, "%s%s", disptxt [ 8 ], blit_scalers [ g_scale ].desc_en );
	  strcpy ( disptxt[8], temp );
	}
#else
	if (highres_current==false)
	{
		if(Scale_org)
			sprintf(temp,"%s    True",disptxt[8]);
		else
			sprintf(temp,"%s   False",disptxt[8]);
		strcpy(disptxt[8],temp);
	}
	else
	{
		sprintf(temp,"%sinactive",disptxt[8]);
		strcpy(disptxt[8],temp);
	}
#endif

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

	sprintf(temp,"%s %3d%%",disptxt[10],vol);
	strcpy(disptxt[10],temp);

#ifdef DINGOO
	if      (batt_level() >= 3739)
		sprintf(temp,"%s  (#####)",disptxt[14]);
	else if (batt_level() >= 3707)
		sprintf(temp,"%s  ( ####)",disptxt[14]);
	else if (batt_level() >= 3675)
		sprintf(temp,"%s  (  ###)",disptxt[14]);
    	else if (batt_level() >= 3643)
		sprintf(temp,"%s  (   ##)",disptxt[14]);
	else if (batt_level() >= 3611)
		sprintf(temp,"%s  (    #)",disptxt[14]);
   	else   sprintf(temp,"%s  (     )",disptxt[14]);
	strcpy(disptxt[14],temp);

//	sprintf(temp,"%s  (%5d)",disptxt[15],(batt_level()/10)*10);
//	strcpy(disptxt[15],temp);	
#endif

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

		S9xDisplayString (disptxt[i], GFX.Screen, 640,i*10+64);		
	}

	//show screen shot for snapshot
	if(SaveSlotNum_old != SaveSlotNum)
	{
		strcpy(temp,"Loading...");
		S9xDisplayString (temp, GFX.Screen +280, 640,210/*204*/);
		S9xDeinitUpdate (320, 240);
		char fname[256], ext[8];
		sprintf(ext, ".s0%d", SaveSlotNum);
		strcpy(fname, S9xGetFilename (ext));
		load_screenshot(fname);
		SaveSlotNum_old = SaveSlotNum;
	}
	show_screenshot();
	S9xDeinitUpdate (320, 240);
}

void menu_loop(void)
{
	bool8_32 exit_loop = false;
	char fname[256], ext[8];
	char snapscreen_tmp[17120];

#ifdef CAANOO
	SDL_Joystick* keyssnes = 0;
#else
	uint8 *keyssnes = 0;
#endif

	SaveSlotNum_old = -1;

	Scale_org = Scale;
	highres_current=Settings.SupportHiRes;

	capt_screenshot();
	memcpy(snapscreen_tmp,snapscreen,17120);

	Scale = false;
	Settings.SupportHiRes=FALSE;
	S9xDeinitDisplay();
	S9xInitDisplay(0, 0);

	menu_dispupdate();
	sys_sleep(100000);

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
								S9xDisplayString (fname, GFX.Screen +280, 640,204);
								S9xDeinitUpdate (320, 240);
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
							Scale_org = !Scale_org;
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
				//PANDORA & DINGOO & WIN32 -----------------------------------------------------
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
								S9xDisplayString (fname, GFX.Screen +280, 640,204);
								S9xDeinitUpdate (320, 240);
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
#ifdef PANDORA
						  // rotate through scalers
							if (keyssnes[sfc_key[RIGHT_1]] == SDL_PRESSED)
							{
								do
								{
									g_scale = (blit_scaler_e) ( ( g_scale + 1 ) % bs_max );
								} while ( blit_scalers [ g_scale ].valid == bs_invalid );
							} else if (keyssnes[sfc_key[LEFT_1]] == SDL_PRESSED)
							{
								do
								{
									g_scale = (blit_scaler_e) ( g_scale - 1 );
									if (g_scale < 1) g_scale = (blit_scaler_e)(bs_max-1);
								} while ( blit_scalers [ g_scale ].valid == bs_invalid );
							}
#else
							Scale_org = !Scale_org;
#endif
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
							if (keyssnes[sfc_key[LEFT_1]] == SDL_PRESSED)
								vol -= 10;
							else
							if (keyssnes[sfc_key[RIGHT_1]] == SDL_PRESSED)
								vol += 10;

							if(vol>=100)
								vol = 100;
							else if (vol<=0)
								vol = 0;
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

	Scale = Scale_org;
	Settings.SupportHiRes=highres_current;
	S9xDeinitDisplay();
	S9xInitDisplay(0, 0);
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

	if (highres_current==true)
	{
		//working but in highres mode
		for(int y=yoffset;y<240-yoffset;y+=3) //80,1 //240,3
		{
			s+=22*1/*(Scale_disp!=TRUE)*/;
			for(int x=0;x<640-128*1/*(Scale_disp!=TRUE)*/;x+=6) //107,1 //214,2 //428,4 +42+42
			{
				uint8 *d = GFX.Screen + y*1024 + x; //1024
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
			for(int x=0 ;x<320*2-128*(Scale_disp!=TRUE);x+=3*2) // 640/6=107
			{
				uint8 *d = GFX.Screen + y*320 *2 + x;
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
			uint8 *d = GFX.Screen + y*320*2 + x;
			*d++ = snapscreen[s++];
			*d++ = snapscreen[s++];
		}
	}
}

#ifdef DINGOO
int batt_level(void)
{
	FILE *FP;
	int mvolts;
	char buf[6]={};
	FP = fopen("/proc/jz/battery", "rb");
	if (!FP) return(0);
	fgets(buf, 5, FP);
	fclose(FP);
 	sscanf(buf, "%d", &mvolts);
	return (mvolts);
}

int chk_hold(void)
{
	FILE *FP;
	uint32 hold=0;
	char buf[12]={};
	FP = fopen("/proc/jz/gpio3_pxpin", "rb");
	if (!FP) return(0);
	fgets(buf, 11, FP);
	fclose(FP);
 	sscanf(buf, "%x", &hold);
	if((hold & 0x400000) == 0)
		return (1);
	else
		return (0);
}
#endif

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
				memset(GFX.Screen + 320*y*2+x,0x11,2);
			}	
		}
		
		for(int i=0;i<=16;i++){
			int j=i+line;
			if(j>=20) j-=20;
			S9xDisplayString (disptxt[j], GFX.Screen, 640,i*10+80-ypix);
		}
		
		ypix+=2;
		if(ypix==12) {
			line++;
			ypix=0;
		}
		if(line == 20) line = 0;
		S9xDeinitUpdate (320, 240);
		sys_sleep(3000);
	}
#ifdef CAANOO
	while( SDL_JoystickGetButton(keyssnes, sfc_key[B_1])!=TRUE );
#else
	while(keyssnes[sfc_key[B_1]] != SDL_PRESSED);
#endif

	return;
}

