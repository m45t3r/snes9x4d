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
#ifndef _PORT_H_
#define _PORT_H_

#include <limits.h>
#include <memory.h>
#include <string.h>

#include <sys/types.h>

/* #define PIXEL_FORMAT RGB565 */
#define GFX_MULTI_FORMAT

#ifndef snes9x_types_defined
#define snes9x_types_defined

typedef unsigned char bool8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed char int8;
typedef signed short int16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long bool8_32;
typedef unsigned long uint8_32;
typedef unsigned long uint16_32;
typedef long int8_32;
typedef long int16_32;
#endif // snes9x_types_defined
#include "pixform.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if defined(__cplusplus) || defined(c_plusplus)
#define EXTERN_C extern "C"
#define START_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define EXTERN_C extern
#define START_EXTERN_C
#define END_EXTERN_C
#endif

#define SDD1_DECOMP

#ifndef WIN32
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define _MAX_DIR PATH_MAX
#define _MAX_DRIVE 1
#define _MAX_FNAME PATH_MAX
#define _MAX_EXT PATH_MAX
#define _MAX_PATH PATH_MAX
#endif

#define ZeroMemory(a, b) memset((a), 0, (b))
#define LSB_FIRST
/* #define FAST_LSB_WORD_ACCESS */

void _makepath(char *path, const char *drive, const char *dir, const char *fname, const char *ext);
void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);

EXTERN_C void S9xGenerateSound();

#ifdef FAST_LSB_WORD_ACCESS
#define READ_WORD(s) (*(uint16 *)(s))
#define READ_3WORD(s) (*(uint32 *)(s)&0x00ffffff)
#define READ_DWORD(s) (*(uint32 *)(s))
#define WRITE_WORD(s, d) *(uint16 *)(s) = (d)
#define WRITE_3WORD(s, d) *(uint16 *)(s) = (uint16)(d), *((uint8 *)(s) + 2) = (uint8)((d) >> 16)
#define WRITE_DWORD(s, d) *(uint32 *)(s) = (d)
#else
#define READ_WORD(s) (*(uint8 *)(s) | (*((uint8 *)(s) + 1) << 8))
#define READ_3WORD(s) (*(uint8 *)(s) | (*((uint8 *)(s) + 1) << 8) | (*((uint8 *)(s) + 2) << 16))
#define READ_DWORD(s)                                                                                                  \
	(*(uint8 *)(s) | (*((uint8 *)(s) + 1) << 8) | (*((uint8 *)(s) + 2) << 16) | (*((uint8 *)(s) + 3) << 24))
#define WRITE_WORD(s, d) *(uint8 *)(s) = (uint8)(d), *((uint8 *)(s) + 1) = (uint8)((d) >> 8)
#define WRITE_3WORD(s, d)                                                                                              \
	*(uint8 *)(s) = (uint8)(d), *((uint8 *)(s) + 1) = (uint8)((d) >> 8), *((uint8 *)(s) + 2) = (uint8)((d) >> 16)
#define WRITE_DWORD(s, d)                                                                                              \
	*(uint8 *)(s) = (uint8)(d), *((uint8 *)(s) + 1) = (uint8)((d) >> 8), *((uint8 *)(s) + 2) = (uint8)((d) >> 16), \
		*((uint8 *)(s) + 3) = (uint8)((d) >> 24)
#endif

#define CHECK_SOUND()

#define SLASH_STR "/"
#define SLASH_CHAR '/'

#ifdef __linux
typedef void (*SignalHandler)(int);
#define SIG_PF SignalHandler
#endif

#ifdef __linux
#define TITLE "Snes9X: Linux"
#endif

#ifndef TITLE
#define TITLE "Snes9x"
#endif

#define STATIC static

#endif
