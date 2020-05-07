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
#ifndef _SNES9X_H_
#define _SNES9X_H_

#define VERSION "1.39"

#include <stdio.h>
#include <stdlib.h>

#include "port.h"
#include "65c816.h"
#include "messages.h"

#if defined(USE_GLIDE) && !defined(GFX_MULTI_FORMAT)
#define GFX_MULTI_FORMAT
#endif

#define ROM_NAME_LEN 23

#ifdef ZLIB
#include "zlib.h"
#define STREAM gzFile
#define READ_STREAM(p,l,s) gzread (s,p,l)
#define WRITE_STREAM(p,l,s) gzwrite (s,p,l)
#define OPEN_STREAM(f,m) gzopen (f,m)
#define CLOSE_STREAM(s) gzclose (s)
#else
#define STREAM FILE *
#define READ_STREAM(p,l,s) fread (p,1,l,s)
#define WRITE_STREAM(p,l,s) fwrite (p,1,l,s)
#define OPEN_STREAM(f,m) fopen (f,m)
#define CLOSE_STREAM(s) fclose (s)
#endif


/* SNES screen width and height */
#define SNES_WIDTH		256
#define SNES_HEIGHT		224
#define SNES_HEIGHT_EXTENDED	239
#define IMAGE_WIDTH		(Settings.SupportHiRes ? SNES_WIDTH * 2 : SNES_WIDTH)
#define IMAGE_HEIGHT		(Settings.SupportHiRes ? SNES_HEIGHT_EXTENDED * 2 : SNES_HEIGHT_EXTENDED)

#define SNES_MAX_NTSC_VCOUNTER  262
#define SNES_MAX_PAL_VCOUNTER   312
#define SNES_HCOUNTER_MAX	342
#define SPC700_TO_65C816_RATIO	2
#define AUTO_FRAMERATE		200

// NTSC master clock signal 21.47727MHz
// PPU: master clock / 4
// 1 / PPU clock * 342 -> 63.695us
// 63.695us / (1 / 3.579545MHz) -> 228 cycles per scanline
// From Earth Worm Jim: APU executes an average of 65.14285714 cycles per
// scanline giving an APU clock speed of 1.022731096MHz

// PAL master clock signal 21.28137MHz
// PPU: master clock / 4
// 1 / PPU clock * 342 -> 64.281us
// 64.281us / (1 / 3.546895MHz) -> 228 cycles per scanline.

#define SNES_SCANLINE_TIME (63.695e-6)
#define SNES_CLOCK_SPEED (3579545)

#define SNES_CLOCK_LEN (1.0 / SNES_CLOCK_SPEED)

#ifdef VAR_CYCLES
#define SNES_CYCLES_PER_SCANLINE ((uint32) ((SNES_SCANLINE_TIME / SNES_CLOCK_LEN) * 6 + 0.5))
#else
#define SNES_CYCLES_PER_SCANLINE ((uint32) (SNES_SCANLINE_TIME / SNES_CLOCK_LEN + 0.5))
#endif

#define SNES_APUTIMER2_CYCLEx10000 ((uint32) ((SNES_CYCLES_PER_SCANLINE * 10000L) * (1.0 / 64000.0) / SNES_SCANLINE_TIME + 0.5))

#define SNES_TR_MASK	    (1 << 4)
#define SNES_TL_MASK	    (1 << 5)
#define SNES_X_MASK	    (1 << 6)
#define SNES_A_MASK	    (1 << 7)
#define SNES_RIGHT_MASK	    (1 << 8)
#define SNES_LEFT_MASK	    (1 << 9)
#define SNES_DOWN_MASK	    (1 << 10)
#define SNES_UP_MASK	    (1 << 11)
#define SNES_START_MASK	    (1 << 12)
#define SNES_SELECT_MASK    (1 << 13)
#define SNES_Y_MASK	    (1 << 14)
#define SNES_B_MASK	    (1 << 15)

enum {
    SNES_MULTIPLAYER5,
    SNES_JOYPAD,
    SNES_MOUSE_SWAPPED,
    SNES_MOUSE,
    SNES_SUPERSCOPE,
    SNES_MAX_CONTROLLER_OPTIONS
};

#define DEBUG_MODE_FLAG	    (1 << 0)
#define TRACE_FLAG	    (1 << 1)
#define SINGLE_STEP_FLAG    (1 << 2)
#define BREAK_FLAG	    (1 << 3)
#define SCAN_KEYS_FLAG	    (1 << 4)
#define SAVE_SNAPSHOT_FLAG  (1 << 5)
#define DELAYED_NMI_FLAG    (1 << 6)
#define NMI_FLAG	    (1 << 7)
#define PROCESS_SOUND_FLAG  (1 << 8)
#define FRAME_ADVANCE_FLAG  (1 << 9)
#define DELAYED_NMI_FLAG2   (1 << 10)
#define IRQ_PENDING_FLAG    (1 << 11)

#ifdef VAR_CYCLES
#define ONE_CYCLE 6
#define SLOW_ONE_CYCLE 8
#define TWO_CYCLES 12
#else
#define ONE_CYCLE 1
#define SLOW_ONE_CYCLE 1
#define TWO_CYCLES 2
#endif

//Needed for SA1
#define MEMMAP_BLOCK_SIZE (0x1000)
#define MEMMAP_NUM_BLOCKS (0x1000000 / MEMMAP_BLOCK_SIZE)

struct SCPUState{
    uint32  Flags;
    bool8_32   BranchSkip;
    bool8_32   NMIActive;
    bool8_32   IRQActive;
    bool8_32   WaitingForInterrupt;
    bool8_32   InDMA;
    uint8_32   WhichEvent;
    uint8   *PC;
    uint8   *PCBase;
    uint8   *PCAtOpcodeStart;
    uint8   *WaitAddress;
    uint32  WaitCounter;
    long   Cycles;
    long   NextEvent;
    long   V_Counter;
    long   MemSpeed;
    long   MemSpeedx2;
    long   FastROMSpeed;
    uint32 AutoSaveTimer;
    bool8_32  SRAMModified;
    uint32 NMITriggerPoint;
    bool8_32  BRKTriggered;
    bool8_32  TriedInterleavedMode2;
    uint32 NMICycleCount;
    uint32 IRQCycleCount;
	//Needed for SA1
	bool8_32   Executing;
	bool8_32   Waiting;
	uint8   *BWRAM;
	uint8   *WaitByteAddress1;
    uint8   *WaitByteAddress2;
	uint8   *Map [MEMMAP_NUM_BLOCKS];
    uint8   *WriteMap [MEMMAP_NUM_BLOCKS];
    int16   op1;
    int16   op2;
    int     arithmetic_op;
    int64   sum;
    bool8_32   overflow;
    uint8   VirtualBitmapFormat;
    bool8_32   in_char_dma;
    uint8   variable_bit_pos;


};

#define HBLANK_START_EVENT 0
#define HBLANK_END_EVENT 1
#define HTIMER_BEFORE_EVENT 2
#define HTIMER_AFTER_EVENT 3
#define NO_EVENT 4

struct SSettings{
    // CPU options
    bool8_32  APUEnabled;
    bool8_32  Shutdown;
    uint8  SoundSkipMethod;
    long   H_Max;
    long   HBlankStart;
    long   CyclesPercentage;
    bool8_32  DisableIRQ;
    bool8_32  Paused;
    bool8_32  ForcedPause;
    bool8_32  StopEmulation;

    // Tracing options
    bool8_32  TraceDMA;
    bool8_32  TraceHDMA;
    bool8_32  TraceVRAM;
    bool8_32  TraceUnknownRegisters;
    bool8_32  TraceDSP;

    // Joystick options
    bool8_32  SwapJoypads;
    bool8_32  JoystickEnabled;

    // ROM timing options (see also H_Max above)
    bool8_32  ForcePAL;
    bool8_32  ForceNTSC;
    bool8_32  PAL;
    uint32 FrameTimePAL;
    uint32 FrameTimeNTSC;
    uint32 FrameTime;
    uint32 SkipFrames;

    // ROM image options
    bool8_32  ForceLoROM;
    bool8_32  ForceHiROM;
    bool8_32  ForceHeader;
    bool8_32  ForceNoHeader;
    bool8_32  ForceInterleaved;
    bool8_32  ForceInterleaved2;
    bool8_32  ForceNotInterleaved;

    // Peripherial options
    bool8_32  ForceSuperFX;
    bool8_32  ForceNoSuperFX;
    bool8_32  ForceDSP1;
    bool8_32  ForceNoDSP1;
    bool8_32  ForceSA1;
    bool8_32  ForceNoSA1;
    bool8_32  ForceC4;
    bool8_32  ForceNoC4;
    bool8_32  ForceSDD1;
    bool8_32  ForceNoSDD1;
    bool8_32  MultiPlayer5;
    bool8_32  Mouse;
    bool8_32  SuperScope;
    bool8_32  SRTC;
    uint32 ControllerOption;
    
    bool8_32  ShutdownMaster;
    bool8_32  MultiPlayer5Master;
    bool8_32  SuperScopeMaster;
    bool8_32  MouseMaster;
    bool8_32  SuperFX;
    bool8_32  DSP1Master;
    bool8_32  SA1;
    bool8_32  C4;
    bool8_32  SDD1;
//	bool8_32  SPC7110;
//	bool8_32  SPC7110RTC;
	bool8_32  OBC1;

    // Sound options
    uint32 SoundPlaybackRate;
    bool8_32  TraceSoundDSP;
    bool8_32  Stereo;
    bool8_32  ReverseStereo;
    bool8_32  SixteenBitSound;
    int    SoundBufferSize;
    int    SoundMixInterval;
    bool8_32  SoundEnvelopeHeightReading;
    bool8_32  DisableSoundEcho;
    bool8_32  DisableSampleCaching;
    bool8_32  DisableMasterVolume;
    bool8_32  SoundSync;
    bool8_32  InterpolatedSound;
    bool8_32  ThreadSound;
    bool8_32  Mute;
    bool8_32  NextAPUEnabled;
    uint8  AltSampleDecode;
    bool8_32  FixFrequency;
    
    // Graphics options
    bool8_32  SixteenBit;
    bool8_32  Transparency;
    bool8_32  SupportHiRes;
    bool8_32  Mode7Interpolate;

    // SNES graphics options
    bool8_32  BGLayering;
    bool8_32  DisableGraphicWindows;
    bool8_32  ForceTransparency;
    bool8_32  ForceNoTransparency;
    bool8_32  DisableHDMA;
    bool8_32  DisplayFrameRate;

    // Others
    bool8_32  NetPlay;
    bool8_32  NetPlayServer;
    char   ServerName [128];
    int    Port;
    bool8_32  GlideEnable;
    bool8_32  OpenGLEnable;
    int32  AutoSaveDelay; // Time in seconds before S-RAM auto-saved if modified.
    bool8_32  ApplyCheats;
    bool8_32  TurboMode;
    uint32 TurboSkipFrames;
    uint32 AutoMaxSkipFrames;
    
// Fixes for individual games
    uint32 StrikeGunnerOffsetHack;
    bool8_32  ChuckRock;
    bool8_32  StarfoxHack;
    bool8_32  WinterGold;
    bool8_32  Dezaemon;
    bool8_32  WrestlemaniaArcade;
    bool8_32  BS;	// Japanese Satellite System games.
    bool8_32  DaffyDuck;
    uint8  APURAMInitialValue;
    
#ifdef __WIN32__
    int    SoundDriver;
#endif
};

struct SSNESGameFixes
{
    uint8 NeedInit0x2137;
    uint8 umiharakawaseFix;
    uint8 alienVSpredetorFix;
    uint8 APU_OutPorts_ReturnValueFix;
    uint8 Old_Read0x4200;
    uint8 _0x213E_ReturnValue;
    uint8 TouhaidenControllerFix;
    uint8 SoundEnvelopeHeightReading2;
    uint8 SRAMInitialValue;
	uint8 Uniracers;
};

START_EXTERN_C
extern struct SSettings Settings;
extern struct SCPUState CPU;
extern struct SSNESGameFixes SNESGameFixes;
extern char String [513];

void S9xExit ();
void S9xMessage (int type, int number, const char *message);
void S9xLoadSDD1Data ();
END_EXTERN_C

enum {
    PAUSE_NETPLAY_CONNECT = (1 << 0),
    PAUSE_TOGGLE_FULL_SCREEN = (1 << 1),
    PAUSE_EXIT = (1 << 2),
    PAUSE_MENU = (1 << 3),
    PAUSE_INACTIVE_WINDOW = (1 << 4),
    PAUSE_WINDOW_ICONISED = (1 << 5),
    PAUSE_RESTORE_GUI = (1 << 6),
    PAUSE_FREEZE_FILE = (1 << 7)
};
void S9xSetPause (uint32 mask);
void S9xClearPause (uint32 mask);

#endif
