/*******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 
  (c) Copyright 1996 - 2003 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2003 Matthew Kendora and
                            Brad Jorsch (anomie@users.sourceforge.net)
 

                      
  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and
                            Nach (n-a-c-h@users.sourceforge.net)
                                          
  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2003 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman (jweidman@slip.net),
                            neviksti (neviksti@hotmail.com), and
                            Kris Bleakley (stinkfish@bigpond.com)
 
  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2003 zsKnight, pagefault (pagefault@zsnes.com)
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-RTC C emulator code
  (c) Copyright 2001 John Weidman
  
  Super FX x86 assembler emulator code 
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault 

  Super FX C emulator code 
  (c) Copyright 1997 - 1999 Ivar and Gary Henderson.



 
  Specific ports contains the works of other authors. See headers in
  individual files.
 
  Snes9x homepage: http://www.snes9x.com
 
  Permission to use, copy, modify and distribute Snes9x in both binary and
  source form, for non-commercial purposes, is hereby granted without fee,
  providing that this license information and copyright notice appear with
  all copies and any derived work.
 
  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software.
 
  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes
  charging money for Snes9x or software derived from Snes9x.
 
  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.
 
  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
*******************************************************************************/

#include <string.h>
#include "memmap.h"
#include "obc1.h"

//#define OBC1_DEBUG

#define OBC1_REG_FLIP  0x0000
#define OBC1_OAM_ADDR  0x0008

static uint8 *obc1ram = NULL;

#ifdef OBC1_DEBUG
const static char obc1_log_name[] = "obc1.log";
static FILE *obc1_fs = NULL;
#endif

// Banks 00->3f and 80->bf
// 6000h-7FFFh OBC1 RAM register.

/* ZSNES comment.
; Affected values: 0,1,2,3,4,6,7,9,A,B
; 0,1 - Another X value (unused?)
; 2   - OAM value
; 3/4 - X value (bits 0-8)
; 5   - N/A (not written to)
; 6   - OAM #
; 7   - Always 0?
; 9   - Y value (bits 0-7)
; A   - OAM value
; B   - OAM Status
; X,Y,OAM,Attr / xhighbit / OAM highbit / Sprite size
;bit 0 = OAM b8, bit 1-3 = palette number bit 4,5 = playfield priority
;bit 6   = horizontal flip bit 7   = horizonal flip
; Extra: bit 0 = X bit 8, bit 1 = Larger sprite size


; SNES PPU 110h Word OAM Ram format.
; 000h-0FFh
; 4 bytes
; 0byte X
; 1byte Y
; 
; 3-4 bytes
; 0-8 bits pattern number
; 9-11 bits palette number
; 12-13 bits priority bits
; 14 bit H-Flip
; 15 bit V-Flip
; 
; 100h-10Fh
; bit0 X pos MSB
; bit1 sprite size (x4)
; 2bit * 128 == 10h word.
*/
extern "C"
{
uint8 GetOBC1 (uint16 Address)
{
#ifdef OBC1_DEBUG
	fprintf(obc1_fs, "0x%04X r 0x%02X\n", Address, obc1ram[Address & 0x1FFF]);
#endif
	return obc1ram[Address & 0x1FFF];
}

void SetOBC1 (uint8 Byte, uint16 Address)
{
	switch(Address) {
		case 0x7FF0:
		{
			obc1ram[0x1FF0] = Byte;
			obc1ram[READ_WORD(&Memory.FillRAM[OBC1_OAM_ADDR]) + 0x00] = Byte;
			
			obc1ram[((uint16)obc1ram[0x1FF6] << 4) + Memory.FillRAM[OBC1_REG_FLIP + 0] + 0] = Byte;
			Memory.FillRAM[OBC1_REG_FLIP + 0] ^= 0x08;
			break;
		}
		
		case 0x7FF1:
		{
			obc1ram[0x1FF1] = Byte;
			obc1ram[READ_WORD(&Memory.FillRAM[OBC1_OAM_ADDR]) + 0x01] = Byte;
			
			obc1ram[((uint16)obc1ram[0x1FF6] << 4) + Memory.FillRAM[OBC1_REG_FLIP + 1] + 1] = Byte;
			Memory.FillRAM[OBC1_REG_FLIP + 1] ^= 0x08;
			break;
		}
		
		case 0x7FF2:
		{
			obc1ram[0x1FF2] = Byte;
			obc1ram[READ_WORD(&Memory.FillRAM[OBC1_OAM_ADDR]) + 0x02] = Byte;
			
			obc1ram[((uint16)obc1ram[0x1FF6] << 4) + Memory.FillRAM[OBC1_REG_FLIP + 2] + 2] = Byte;
			Memory.FillRAM[OBC1_REG_FLIP + 2] ^= 0x08;
			break;
		}
		
		case 0x7FF3:
		{
			obc1ram[0x1FF3] = Byte;
			if(!Memory.FillRAM[OBC1_REG_FLIP + 3]) {
				obc1ram[READ_WORD(&Memory.FillRAM[OBC1_OAM_ADDR]) + 0x00] = Byte;
			}
			else {
				obc1ram[READ_WORD(&Memory.FillRAM[OBC1_OAM_ADDR]) + 0x03] = Byte;
			}
			
			obc1ram[((uint16)obc1ram[0x1FF6] << 4) + Memory.FillRAM[OBC1_REG_FLIP + 3] + 3] = Byte;
			Memory.FillRAM[OBC1_REG_FLIP + 3] ^= 0x08;
			break;
		}
		
		case 0x7FF4:
		{
			obc1ram[0x1FF4] = Byte;
			uint16 oami = 0x1800 + 0x200 + ((uint16)obc1ram[0x1FF6] >> 2);
			uint8 oams = ((obc1ram[0x1FF6] & 3) << 1);
			obc1ram[oami] = (obc1ram[oami] & ~(3 << oams)) | ((Byte & 3) << oams);
			
			obc1ram[((uint16)obc1ram[0x1FF6] << 4) + Memory.FillRAM[OBC1_REG_FLIP + 4] + 4] = Byte;
			Memory.FillRAM[OBC1_REG_FLIP + 4] ^= 0x08;
			break;
		}
		
		case 0x7FF5:
		{
			obc1ram[0x1FF5] = Byte;
			
			obc1ram[((uint16)obc1ram[0x1FF6] << 4) + Memory.FillRAM[OBC1_REG_FLIP + 5] + 5] = Byte;
			Memory.FillRAM[OBC1_REG_FLIP + 5] ^= 0x08;
			break;
		}
		
		case 0x7FF6:
		{
			Byte &= 0x7F;
			obc1ram[0x1FF6] = Byte;
			WRITE_WORD(&Memory.FillRAM[OBC1_OAM_ADDR], 0x1800 + (Byte << 2));
			*((uint32 *)&Memory.FillRAM[OBC1_REG_FLIP]) &= 0xF7F7F7F7;
			*((uint32 *)&Memory.FillRAM[OBC1_REG_FLIP + 4]) &= 0xF7F7F7F7;
			
			uint16 obci = (uint16)obc1ram[0x1FF6] << 4;
			uint16 oami = 0x1800 + (obci >> 2);
			obc1ram[oami + 0x00] = obc1ram[obci + 0x03];
			obc1ram[oami + 0x01] = obc1ram[obci + 0x09];
			obc1ram[oami + 0x02] = obc1ram[obci + 0x0A];
			obc1ram[oami + 0x03] = obc1ram[obci + 0x0B];
			uint16 oamii = 0x1800 + 0x200 + (oami >> 4);
			uint8 oams = ((obc1ram[0x1FF6] & 3) << 1);
			obc1ram[oamii] = (obc1ram[oamii] & ~(3 << oams)) | ((obc1ram[obci + 0x04] & 3) << oams);
			
			obc1ram[((uint16)obc1ram[0x1FF6] << 4) + 6] = Byte;
			break;
		}
		
		case 0x7FF7:
		{
			obc1ram[0x1FF7] = Byte;
			
			obc1ram[((uint16)obc1ram[0x1FF6] << 4) + Memory.FillRAM[OBC1_REG_FLIP + 7] + 7] = Byte;
			Memory.FillRAM[OBC1_REG_FLIP + 7] ^= 0x08;
			break;
		}
		
		default:
			obc1ram[Address & 0x1FFF] = Byte;
			break;
	}
#ifdef OBC1_DEBUG
	fprintf(obc1_fs, "0x%04X w 0x%02X\n", Address, Byte);
#endif
	return;
}

uint8 *GetBasePointerOBC1(uint32 Address)
{
	return Memory.FillRAM;
}

uint8 *GetMemPointerOBC1(uint32 Address)
{
	return (Memory.FillRAM + (Address & 0xFFFF));
}

void ResetOBC1()//bool8 full)
{
	obc1ram = &Memory.FillRAM[0x6000];
	memset(obc1ram, 0x00, 0x2000);
	memset(&Memory.FillRAM[OBC1_REG_FLIP], 0x00, 8);
	WRITE_WORD(&Memory.FillRAM[OBC1_OAM_ADDR], 0x1800);
#ifdef OBC1_DEBUG
	if(!obc1_fs)
		obc1_fs = fopen(obc1_log_name, "w");
#endif
	return;
}

}
