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
#ifndef _SPC700_H_
#define _SPC700_H_

#ifdef SPCTOOL
#define NO_CHANNEL_STRUCT
//#include "spctool/dsp.h"
//#include "spctool/spc700.h"
//#include "spctool/soundmod.h"
#endif

#define Carry       1
#define Zero        2
#define Interrupt   4
#define HalfCarry   8
#define BreakFlag  16
#define DirectPageFlag 32
#define Overflow   64
#define Negative  128

#define APUClearCarry() (iapu->_Carry = 0)
#define APUSetCarry() (iapu->_Carry = 1)
#define APUSetInterrupt() (areg->P |= Interrupt)
#define APUClearInterrupt() (areg->P &= ~Interrupt)
#define APUSetHalfCarry() (areg->P |= HalfCarry)
#define APUClearHalfCarry() (areg->P &= ~HalfCarry)
#define APUSetBreak() (areg->P |= BreakFlag)
#define APUClearBreak() (areg->P &= ~BreakFlag)
#define APUSetDirectPage() (areg->P |= DirectPageFlag)
#define APUClearDirectPage() (areg->P &= ~DirectPageFlag)
#define APUSetOverflow() (iapu->_Overflow = 1)
#define APUClearOverflow() (iapu->_Overflow = 0)

#define APUCheckZero() (iapu->_Zero == 0)
#define APUCheckCarry() (iapu->_Carry)
#define APUCheckInterrupt() (areg->P & Interrupt)
#define APUCheckHalfCarry() (areg->P & HalfCarry)
#define APUCheckBreak() (areg->P & BreakFlag)
#define APUCheckDirectPage() (APURegisters.P & DirectPageFlag)
#define APUCHECKDIRECTPAGE_OP() (APURegisters.P & DirectPageFlag)
#define APUCheckOverflow() (iapu->_Overflow)
#define APUCheckNegative() (iapu->_Zero & 0x80)

#define APUClearFlags(f) (areg->P &= ~(f))
#define APUSetFlags(f)   (areg->P |=  (f))
#define APUCheckFlag(f)  (areg->P &   (f))

typedef union
{
#ifdef LSB_FIRST
    struct { uint8 A, Y; } B;
#else
    struct { uint8 Y, A; } B;
#endif
    uint16 W;
} YAndA;

struct SAPURegisters{
    uint16_32	PC;
    uint8_32	P;
    YAndA		YA;
    uint8_32	X;
    uint8_32	S;
};

/*
struct SAPURegisters{
    uint8  P;
    YAndA YA;
    uint8  X;
    uint8  S;
    uint16  PC;
};
*/

EXTERN_C struct SAPURegisters APURegisters;

// Needed by ILLUSION OF GAIA
//#define ONE_APU_CYCLE 14
#define ONE_APU_CYCLE 21

// Needed by all games written by the software company called Human
//#define ONE_APU_CYCLE_HUMAN 17
#define ONE_APU_CYCLE_HUMAN 21

// 1.953us := 1.024065.54MHz

#ifdef SPCTOOL
EXTERN_C int32 ESPC (int32);

#define APU_EXECUTE() \
{ \
    int32 l = (CPU.Cycles - APU.Cycles) / 14; \
    if (l > 0) \
    { \
        l -= _EmuSPC(l); \
        APU.Cycles += l * 14; \
    } \
}

#else

#ifdef DEBUGGER
#define APU_EXECUTE1() \
{ \
    if (APU.Flags & TRACE_FLAG) \
	S9xTraceAPU ();\
    APU.Cycles += S9xAPUCycles [*IAPU.PC]; \
    (*S9xApuOpcodes[*IAPU.PC]) (&APURegisters, &IAPU, &APU); \
}
#else
#define APU_EXECUTE1() \
{ \
    apu->Cycles += S9xAPUCycles [*iapu->PC]; \
    (*S9xApuOpcodes[*iapu->PC]) (&APURegisters, iapu, apu); \
}
#endif

#define APU_EXECUTE() \
if (iapu->APUExecuting) \
{\
    while (apu->Cycles <= cpu->Cycles) \
	APU_EXECUTE1(); \
}
#endif

#endif
