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
* SPC7110 code (c) Copyright 2002 Matthew Kendora, Dark Force, zsKnight,
*						and John Weidman, with fixes and debugging code
*						from CaitSith2
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
#ifndef _spc7110_h
#define _spc7110_h
#include "port.h"

#define DECOMP_BUFFER_SIZE	0x10000

extern void (*LoadUp7110)(char*);
extern void (*CleanUp7110)(void);
extern void (*Copy7110)(void);

extern uint16 cacheMegs;

void Del7110Gfx(void);
void Close7110Gfx(void);
void Drop7110Gfx(void);
extern "C"{
uint8 S9xGetSPC7110(uint16 Address);
uint8 S9xGetS7110Byte(uint32 Address);
uint8* Get7110BasePtr(uint32);
}
void S9xSetSPC7110 (uint8 data, uint16 Address);
void S9xSpc7110Init();
uint8* Get7110BasePtr(uint32);
void S9xSpc7110Reset();
void S9xUpdateRTC ();
void Do7110Logging();
int	S9xRTCDaysInMonth( int month, int year );

//These are platform-dependant functions, but should work on
//most systems that use GNU compilers, and on Win32.
void SPC7110Load(char*);
void SPC7110Open(char*);
void SPC7110Grab(char*);

typedef struct SPC7110RTC
{
	unsigned char reg[16];
	short index;
	uint8 control;
	bool init;
	time_t last_used;
} S7RTC;

typedef struct SPC7110EmuVars
{
	unsigned char reg4800;
	unsigned char reg4801;
	unsigned char reg4802;
	unsigned char reg4803;
	unsigned char reg4804;
	unsigned char reg4805;
	unsigned char reg4806;
	unsigned char reg4807;
	unsigned char reg4808;
	unsigned char reg4809;
	unsigned char reg480A;
	unsigned char reg480B;
	unsigned char reg480C;
	unsigned char reg4811;
	unsigned char reg4812;
	unsigned char reg4813;
	unsigned char reg4814;
	unsigned char reg4815;
	unsigned char reg4816;
	unsigned char reg4817;
	unsigned char reg4818;
	unsigned char reg4820;
	unsigned char reg4821;
	unsigned char reg4822;
	unsigned char reg4823;
	unsigned char reg4824;
	unsigned char reg4825;
	unsigned char reg4826;
	unsigned char reg4827;
	unsigned char reg4828;
	unsigned char reg4829;
	unsigned char reg482A;
	unsigned char reg482B;
	unsigned char reg482C;
	unsigned char reg482D;
	unsigned char reg482E;
	unsigned char reg482F;
	unsigned char reg4830;
	unsigned char reg4831;
	unsigned char reg4832;
	unsigned char reg4833;
	unsigned char reg4834;
	unsigned char reg4840;
	unsigned char reg4841;
	unsigned char reg4842;
	uint8 AlignBy;
	uint8 written;
	uint8 offset_add;
	uint32 DataRomOffset;
	uint32 DataRomSize;
	uint32 bank50Internal;
	uint8 bank50[DECOMP_BUFFER_SIZE];

} SPC7110Regs;
extern SPC7110Regs s7r;
extern S7RTC rtc_f9;
#endif
