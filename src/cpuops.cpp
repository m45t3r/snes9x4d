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
/**********************************************************************************************/
/* CPU-S9xOpcodes.CPP                                                                            */
/* This file contains all the opcodes                                                         */
/**********************************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "debug.h"
#include "missing.h"
#include "apu.h"
#include "sa1.h"

START_EXTERN_C
/*
extern uint8 A1, A2, A3, A4, W1, W2, W3, W4;
extern uint8 Ans8;
extern uint16 Ans16;
extern uint32 Ans32;
extern uint8 Work8;
extern uint16 Work16;
extern uint32 Work32;
extern signed char Int8;
extern short Int16;
extern long Int32;
*/
END_EXTERN_C

#include "cpuexec.h"
#include "cpuaddr.h"
#include "cpuops.h"
#include "cpumacro.h"
#include "apu.h"

/* ADC *************************************************************************************** */
static void Op69M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Immediate8 (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op69M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Immediate16 (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op65M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op65M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op75M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op75M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op72M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op72M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op61M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op61M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op71M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op71M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op67M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op67M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op77M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op77M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op6DM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op6DM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op7DM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op7DM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op79M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op79M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op6FM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op6FM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op7FM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op7FM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op63M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op63M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

static void Op73M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    ADC8 (OpAddress, reg, icpu, cpu);
}

static void Op73M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    ADC16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/

/* AND *************************************************************************************** */
static void Op29M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->AL &= *cpu->PC++;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
    SETZN8 (reg->AL);
}

static void Op29M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    reg->A.W &= *(uint16 *) cpu->PC;
#else
    reg->A.W &= *cpu->PC + (*(cpu->PC + 1) << 8);
#endif
    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    SETZN16 (reg->A.W);
}

static void Op25M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op25M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op35M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op35M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op32M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op32M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op21M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op21M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op31M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op31M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op27M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op27M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op37M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op37M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op2DM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op2DM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op3DM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op3DM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op39M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op39M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op2FM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op2FM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op3FM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op3FM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op23M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op23M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}

static void Op33M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    AND8 (OpAddress, reg, icpu, cpu);
}

static void Op33M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    AND16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* ASL *************************************************************************************** */
static void Op0AM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_ASL8 (reg, icpu, cpu);
}

static void Op0AM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_ASL16 (reg, icpu, cpu);
}

static void Op06M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    ASL8 (OpAddress, reg, icpu, cpu);
}

static void Op06M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    ASL16 (OpAddress, reg, icpu, cpu);
}

static void Op16M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    ASL8 (OpAddress, reg, icpu, cpu);
}

static void Op16M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    ASL16 (OpAddress, reg, icpu, cpu);
}

static void Op0EM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    ASL8 (OpAddress, reg, icpu, cpu);
}

static void Op0EM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    ASL16 (OpAddress, reg, icpu, cpu);
}

static void Op1EM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    ASL8 (OpAddress, reg, icpu, cpu);
}

static void Op1EM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    ASL16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* BIT *************************************************************************************** */
static void Op89M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    icpu->_Zero = reg->AL & *cpu->PC++;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
}

static void Op89M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    icpu->_Zero = (reg->A.W & *(uint16 *) cpu->PC) != 0;
#else
    icpu->_Zero = (reg->A.W & (*cpu->PC + (*(cpu->PC + 1) << 8))) != 0;
#endif	
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    cpu->PC += 2;
}

static void Op24M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    BIT8 (OpAddress, reg, icpu, cpu);
}

static void Op24M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    BIT16 (OpAddress, reg, icpu, cpu);
}

static void Op34M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    BIT8 (OpAddress, reg, icpu, cpu);
}

static void Op34M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    BIT16 (OpAddress, reg, icpu, cpu);
}

static void Op2CM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    BIT8 (OpAddress, reg, icpu, cpu);
}

static void Op2CM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    BIT16 (OpAddress, reg, icpu, cpu);
}

static void Op3CM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    BIT8 (OpAddress, reg, icpu, cpu);
}

static void Op3CM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    BIT16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* CMP *************************************************************************************** */
static void OpC9M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    int32 Int32 = (int) reg->AL - (int) *cpu->PC++;
    icpu->_Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
}

static void OpC9M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS    
    int32 Int32 = (long) reg->A.W - (long) *(uint16 *) cpu->PC;
#else
    int32 Int32 = (long) reg->A.W -
	    (long) (*cpu->PC + (*(cpu->PC + 1) << 8));
#endif
    icpu->_Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
}

static void OpC5M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpC5M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpD5M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpD5M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpD2M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpD2M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpC1M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpC1M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpD1M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpD1M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpC7M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpC7M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpD7M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpD7M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpCDM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpCDM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpDDM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpDDM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpD9M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpD9M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpCFM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpCFM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpDFM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpDFM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpC3M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpC3M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

static void OpD3M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    CMP8 (OpAddress, reg, icpu, cpu);
}

static void OpD3M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    CMP16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/

/* CMX *************************************************************************************** */
static void OpE0X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    int32 Int32 = (int) reg->XL - (int) *cpu->PC++;
    icpu->_Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
}

static void OpE0X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS    
    int32 Int32 = (long) reg->X.W - (long) *(uint16 *) cpu->PC;
#else
    int32 Int32 = (long) reg->X.W -
	    (long) (*cpu->PC + (*(cpu->PC + 1) << 8));
#endif
    icpu->_Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
}

static void OpE4X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    CMX8 (OpAddress, reg, icpu, cpu);
}

static void OpE4X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    CMX16 (OpAddress, reg, icpu, cpu);
}

static void OpECX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    CMX8 (OpAddress, reg, icpu, cpu);
}

static void OpECX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    CMX16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/

/* CMY *************************************************************************************** */
static void OpC0X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    int32 Int32 = (int) reg->YL - (int) *cpu->PC++;
    icpu->_Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
}

static void OpC0X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS    
    int32 Int32 = (long) reg->Y.W - (long) *(uint16 *) cpu->PC;
#else
    int32 Int32 = (long) reg->Y.W -
	    (long) (*cpu->PC + (*(cpu->PC + 1) << 8));
#endif
    icpu->_Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
}

static void OpC4X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    CMY8 (OpAddress, reg, icpu, cpu);
}

static void OpC4X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    CMY16 (OpAddress, reg, icpu, cpu);
}

static void OpCCX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    CMY8 (OpAddress, reg, icpu, cpu);
}

static void OpCCX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    CMY16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/

/* DEC *************************************************************************************** */
static void Op3AM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_DEC8 (reg, icpu, cpu);
}

static void Op3AM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_DEC16 (reg, icpu, cpu);
}

static void OpC6M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    DEC8 (OpAddress, reg, icpu, cpu);
}

static void OpC6M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    DEC16 (OpAddress, reg, icpu, cpu);
}

static void OpD6M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    DEC8 (OpAddress, reg, icpu, cpu);
}

static void OpD6M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    DEC16 (OpAddress, reg, icpu, cpu);
}

static void OpCEM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    DEC8 (OpAddress, reg, icpu, cpu);
}

static void OpCEM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    DEC16 (OpAddress, reg, icpu, cpu);
}

static void OpDEM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    DEC8 (OpAddress, reg, icpu, cpu);
}

static void OpDEM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    DEC16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/

/* EOR *************************************************************************************** */
static void Op49M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->AL ^= *cpu->PC++;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
    SETZN8 (reg->AL);
}

static void Op49M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    reg->A.W ^= *(uint16 *) cpu->PC;
#else
    reg->A.W ^= *cpu->PC + (*(cpu->PC + 1) << 8);
#endif
    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    SETZN16 (reg->A.W);
}

static void Op45M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op45M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op55M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op55M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op52M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op52M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op41M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op41M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op51M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op51M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op47M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op47M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op57M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op57M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op4DM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op4DM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op5DM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op5DM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op59M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op59M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op4FM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op4FM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op5FM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op5FM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op43M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op43M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

static void Op53M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    EOR8 (OpAddress, reg, icpu, cpu);
}

static void Op53M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    EOR16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/

/* INC *************************************************************************************** */
static void Op1AM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_INC8 (reg, icpu, cpu);
}

static void Op1AM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_INC16 (reg, icpu, cpu);
}

static void OpE6M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    INC8 (OpAddress, reg, icpu, cpu);
}

static void OpE6M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    INC16 (OpAddress, reg, icpu, cpu);
}

static void OpF6M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    INC8 (OpAddress, reg, icpu, cpu);
}

static void OpF6M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    INC16 (OpAddress, reg, icpu, cpu);
}

static void OpEEM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    INC8 (OpAddress, reg, icpu, cpu);
}

static void OpEEM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    INC16 (OpAddress, reg, icpu, cpu);
}

static void OpFEM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    INC8 (OpAddress, reg, icpu, cpu);
}

static void OpFEM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    INC16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/
/* LDA *************************************************************************************** */
static void OpA9M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->AL = *cpu->PC++;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
    SETZN8 (reg->AL);
}

static void OpA9M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    reg->A.W = *(uint16 *) cpu->PC;
#else
    reg->A.W = *cpu->PC + (*(cpu->PC + 1) << 8);
#endif

    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    SETZN16 (reg->A.W);
}

static void OpA5M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpA5M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpB5M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpB5M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpB2M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpB2M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpA1M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpA1M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpB1M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpB1M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpA7M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpA7M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpB7M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpB7M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpADM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpADM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpBDM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpBDM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpB9M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpB9M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpAFM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpAFM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpBFM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpBFM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpA3M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpA3M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

static void OpB3M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    LDA8 (OpAddress, reg, icpu, cpu);
}

static void OpB3M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    LDA16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/

/* LDX *************************************************************************************** */
static void OpA2X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->XL = *cpu->PC++;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
    SETZN8 (reg->XL);
}

static void OpA2X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    reg->X.W = *(uint16 *) cpu->PC;
#else
    reg->X.W = *cpu->PC + (*(cpu->PC + 1) << 8);
#endif
    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    SETZN16 (reg->X.W);
}

static void OpA6X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    LDX8 (OpAddress, reg, icpu, cpu);
}

static void OpA6X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    LDX16 (OpAddress, reg, icpu, cpu);
}

static void OpB6X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedY (reg, icpu, cpu);
    LDX8 (OpAddress, reg, icpu, cpu);
}

static void OpB6X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedY (reg, icpu, cpu);
    LDX16 (OpAddress, reg, icpu, cpu);
}

static void OpAEX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    LDX8 (OpAddress, reg, icpu, cpu);
}

static void OpAEX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    LDX16 (OpAddress, reg, icpu, cpu);
}

static void OpBEX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    LDX8 (OpAddress, reg, icpu, cpu);
}

static void OpBEX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    LDX16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* LDY *************************************************************************************** */
static void OpA0X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->YL = *cpu->PC++;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
    SETZN8 (reg->YL);
}

static void OpA0X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    reg->Y.W = *(uint16 *) cpu->PC;
#else
    reg->Y.W = *cpu->PC + (*(cpu->PC + 1) << 8);
#endif

    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    SETZN16 (reg->Y.W);
}

static void OpA4X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    LDY8 (OpAddress, reg, icpu, cpu);
}

static void OpA4X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    LDY16 (OpAddress, reg, icpu, cpu);
}

static void OpB4X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    LDY8 (OpAddress, reg, icpu, cpu);
}

static void OpB4X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    LDY16 (OpAddress, reg, icpu, cpu);
}

static void OpACX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    LDY8 (OpAddress, reg, icpu, cpu);
}

static void OpACX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    LDY16 (OpAddress, reg, icpu, cpu);
}

static void OpBCX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    LDY8 (OpAddress, reg, icpu, cpu);
}

static void OpBCX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    LDY16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* LSR *************************************************************************************** */
static void Op4AM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_LSR8 (reg, icpu, cpu);
}

static void Op4AM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_LSR16 (reg, icpu, cpu);
}

static void Op46M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    LSR8 (OpAddress, reg, icpu, cpu);
}

static void Op46M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    LSR16 (OpAddress, reg, icpu, cpu);
}

static void Op56M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    LSR8 (OpAddress, reg, icpu, cpu);
}

static void Op56M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    LSR16 (OpAddress, reg, icpu, cpu);
}

static void Op4EM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    LSR8 (OpAddress, reg, icpu, cpu);
}

static void Op4EM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    LSR16 (OpAddress, reg, icpu, cpu);
}

static void Op5EM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    LSR8 (OpAddress, reg, icpu, cpu);
}

static void Op5EM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    LSR16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/

/* ORA *************************************************************************************** */
static void Op09M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->AL |= *cpu->PC++;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
    SETZN8 (reg->AL);
}

static void Op09M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    reg->A.W |= *(uint16 *) cpu->PC;
#else
    reg->A.W |= *cpu->PC + (*(cpu->PC + 1) << 8);
#endif
    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    SETZN16 (reg->A.W);
}

static void Op05M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op05M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op15M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op15M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op12M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op12M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op01M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op01M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op11M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op11M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op07M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op07M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op17M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op17M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op0DM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op0DM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op1DM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op1DM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op19M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op19M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op0FM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op0FM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op1FM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op1FM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op03M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op03M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

static void Op13M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    ORA8 (OpAddress, reg, icpu, cpu);
}

static void Op13M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    ORA16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/

/* ROL *************************************************************************************** */
static void Op2AM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_ROL8 (reg, icpu, cpu);
}

static void Op2AM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_ROL16 (reg, icpu, cpu);
}

static void Op26M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    ROL8 (OpAddress, reg, icpu, cpu);
}

static void Op26M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    ROL16 (OpAddress, reg, icpu, cpu);
}

static void Op36M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    ROL8 (OpAddress, reg, icpu, cpu);
}

static void Op36M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    ROL16 (OpAddress, reg, icpu, cpu);
}

static void Op2EM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    ROL8 (OpAddress, reg, icpu, cpu);
}

static void Op2EM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    ROL16 (OpAddress, reg, icpu, cpu);
}

static void Op3EM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    ROL8 (OpAddress, reg, icpu, cpu);
}

static void Op3EM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    ROL16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* ROR *************************************************************************************** */
static void Op6AM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_ROR8 (reg, icpu, cpu);
}

static void Op6AM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    A_ROR16 (reg, icpu, cpu);
}

static void Op66M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    ROR8 (OpAddress, reg, icpu, cpu);
}

static void Op66M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    ROR16 (OpAddress, reg, icpu, cpu);
}

static void Op76M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    ROR8 (OpAddress, reg, icpu, cpu);
}

static void Op76M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    ROR16 (OpAddress, reg, icpu, cpu);
}

static void Op6EM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    ROR8 (OpAddress, reg, icpu, cpu);
}

static void Op6EM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    ROR16 (OpAddress, reg, icpu, cpu);
}

static void Op7EM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    ROR8 (OpAddress, reg, icpu, cpu);
}

static void Op7EM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    ROR16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* SBC *************************************************************************************** */
static void OpE9M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Immediate8 (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpE9M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Immediate16 (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpE5M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpE5M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpF5M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpF5M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpF2M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpF2M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpE1M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpE1M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpF1M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpF1M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpE7M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpE7M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpF7M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpF7M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpEDM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpEDM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpFDM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpFDM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpF9M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpF9M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpEFM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpEFM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpFFM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpFFM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpE3M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpE3M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}

static void OpF3M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    SBC8 (OpAddress, reg, icpu, cpu);
}

static void OpF3M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    SBC16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* STA *************************************************************************************** */
static void Op85M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op85M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op95M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op95M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op92M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op92M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op81M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
#ifdef noVAR_CYCLES
    if (CHECKINDEX ())
	cpu->Cycles += ONE_CYCLE;
#endif
}

static void Op81M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedIndirect (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
#ifdef noVAR_CYCLES
    if (CHECKINDEX ())
	cpu->Cycles += ONE_CYCLE;
#endif
}

static void Op91M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op91M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexed (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op87M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op87M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectLong (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op97M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op97M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirectIndexedLong (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op8DM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op8DM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op9DM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op9DM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op99M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op99M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedY (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op8FM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op8FM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op9FM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op9FM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLongIndexedX (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op83M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op83M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelative (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}

static void Op93M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    STA8 (OpAddress, reg, icpu, cpu);
}

static void Op93M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = StackRelativeIndirectIndexed (reg, icpu, cpu);
    STA16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* STX *************************************************************************************** */
static void Op86X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    STX8 (OpAddress, reg, icpu, cpu);
}

static void Op86X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    STX16 (OpAddress, reg, icpu, cpu);
}

static void Op96X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedY (reg, icpu, cpu);
    STX8 (OpAddress, reg, icpu, cpu);
}

static void Op96X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedY (reg, icpu, cpu);
    STX16 (OpAddress, reg, icpu, cpu);
}

static void Op8EX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    STX8 (OpAddress, reg, icpu, cpu);
}

static void Op8EX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    STX16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* STY *************************************************************************************** */
static void Op84X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    STY8 (OpAddress, reg, icpu, cpu);
}

static void Op84X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    STY16 (OpAddress, reg, icpu, cpu);
}

static void Op94X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    STY8 (OpAddress, reg, icpu, cpu);
}

static void Op94X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    STY16 (OpAddress, reg, icpu, cpu);
}

static void Op8CX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    STY8 (OpAddress, reg, icpu, cpu);
}

static void Op8CX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    STY16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* STZ *************************************************************************************** */
static void Op64M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    STZ8 (OpAddress, reg, icpu, cpu);
}

static void Op64M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    STZ16 (OpAddress, reg, icpu, cpu);
}

static void Op74M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    STZ8 (OpAddress, reg, icpu, cpu);
}

static void Op74M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndexedX (reg, icpu, cpu);
    STZ16 (OpAddress, reg, icpu, cpu);
}

static void Op9CM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    STZ8 (OpAddress, reg, icpu, cpu);
}

static void Op9CM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    STZ16 (OpAddress, reg, icpu, cpu);
}

static void Op9EM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    STZ8 (OpAddress, reg, icpu, cpu);
}

static void Op9EM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedX (reg, icpu, cpu);
    STZ16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/

/* TRB *************************************************************************************** */
static void Op14M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    TRB8 (OpAddress, reg, icpu, cpu);
}

static void Op14M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    TRB16 (OpAddress, reg, icpu, cpu);
}

static void Op1CM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    TRB8 (OpAddress, reg, icpu, cpu);
}

static void Op1CM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    TRB16 (OpAddress, reg, icpu, cpu);
}
/**********************************************************************************************/

/* TSB *************************************************************************************** */
static void Op04M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    TSB8 (OpAddress, reg, icpu, cpu);
}

static void Op04M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Direct (reg, icpu, cpu);
    TSB16 (OpAddress, reg, icpu, cpu);
}

static void Op0CM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    TSB8 (OpAddress, reg, icpu, cpu);
}

static void Op0CM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    TSB16 (OpAddress, reg, icpu, cpu);
}

/**********************************************************************************************/

/* Branch Instructions *********************************************************************** */
#if !defined(SA1_OPCODES)
#define BranchCheck0()\
    if( cpu->BranchSkip)\
    {\
	cpu->BranchSkip = FALSE;\
	if (!Settings.SoundSkipMethod)\
	    if( cpu->PC - cpu->PCBase > OpAddress)\
	        return;\
    }

#define BranchCheck1()\
    if( cpu->BranchSkip)\
    {\
	cpu->BranchSkip = FALSE;\
	if (!Settings.SoundSkipMethod) {\
	    if( cpu->PC - cpu->PCBase > OpAddress)\
	        return;\
	} else \
	if (Settings.SoundSkipMethod == 1)\
	    return;\
	if (Settings.SoundSkipMethod == 3)\
	    if( cpu->PC - cpu->PCBase > OpAddress)\
	        return;\
	    else\
		cpu->PC = cpu->PCBase + OpAddress;\
    }

#define BranchCheck2()\
    if( cpu->BranchSkip)\
    {\
	cpu->BranchSkip = FALSE;\
	if (!Settings.SoundSkipMethod) {\
	    if( cpu->PC - cpu->PCBase > OpAddress)\
	        return;\
	} else \
	if (Settings.SoundSkipMethod == 1)\
	    cpu->PC = cpu->PCBase + OpAddress;\
	if (Settings.SoundSkipMethod == 3)\
	    if (cpu->PC - cpu->PCBase > OpAddress)\
	        return;\
	    else\
		cpu->PC = cpu->PCBase + OpAddress;\
    }
#else
#define BranchCheck0()
#define BranchCheck1()
#define BranchCheck2()
#endif

#ifdef CPU_SHUTDOWN
#ifndef SA1_OPCODES
inline void CPUShutdown(struct SICPU * icpu, struct SCPUState * cpu)
{
	struct SIAPU *iapu = &IAPU;
	struct SAPU *apu = &APU;

    if (Settings.Shutdown && cpu->PC == cpu->WaitAddress)
    {
	// Don't skip cycles with a pending NMI or IRQ - could cause delayed
	// interrupt. Interrupts are delayed for a few cycles already, but
	// the delay could allow the shutdown code to cycle skip again.
	// Was causing screen flashing on Top Gear 3000.

	if (cpu->WaitCounter == 0 && 
	    !(cpu->Flags & (IRQ_PENDING_FLAG | NMI_FLAG)))
	{
	    cpu->WaitAddress = NULL;
	    if (Settings.SA1)
		S9xSA1ExecuteDuringSleep ();
	    cpu->Cycles = cpu->NextEvent;
   		S9xUpdateAPUTimer();
	    if (IAPU.APUExecuting)
	    {
		icpu->CPUExecuting = FALSE;
		do
		{
		    APU_EXECUTE1();
		} while (apu->Cycles < cpu->NextEvent);
		icpu->CPUExecuting = TRUE;
	    }
	}
	else
	if (cpu->WaitCounter >= 2)
	    cpu->WaitCounter = 1;
	else
	    cpu->WaitCounter--;
    }
}
#else
inline void CPUShutdown(struct SICPU * icpu, struct SCPUState * cpu)
{
    if (Settings.Shutdown && CPU.PC == CPU.WaitAddress)
    {
	if (CPU.WaitCounter >= 1)
	{
	    SA1.Executing = FALSE;
	    SA1ICPU.CPUExecuting = FALSE;
	}
	else
	    CPU.WaitCounter++;
    }
}
#endif
#else
#define CPUShutdown()
#endif

/* BCC */
static void Op90 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Relative (reg, icpu, cpu);
    BranchCheck0 ();
    if (!CHECKCARRY())
    {
	cpu->PC = cpu->PCBase + OpAddress;
#ifdef VAR_CYCLES
	cpu->Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	cpu->Cycles++;
#endif
#endif
	CPUShutdown (icpu, cpu);
    }
}

/* BCS */
static void OpB0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Relative (reg, icpu, cpu);
    BranchCheck0 ();
    if (CHECKCARRY())
    {
	cpu->PC = cpu->PCBase + OpAddress;
#ifdef VAR_CYCLES
	cpu->Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	cpu->Cycles++;
#endif
#endif
	CPUShutdown (icpu, cpu);
    }
}

/* BEQ */
static void OpF0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Relative (reg, icpu, cpu);
    BranchCheck2 ();
    if (CHECKZERO())
    {
	cpu->PC = cpu->PCBase + OpAddress;
#ifdef VAR_CYCLES
	cpu->Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	cpu->Cycles++;
#endif
#endif
	CPUShutdown (icpu, cpu);
    }
}

/* BMI */
static void Op30 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Relative (reg, icpu, cpu);
    BranchCheck1 ();
    if (CHECKNEGATIVE())
    {
	cpu->PC = cpu->PCBase + OpAddress;
#ifdef VAR_CYCLES
	cpu->Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	cpu->Cycles++;
#endif
#endif
	CPUShutdown (icpu, cpu);
    }
}

/* BNE */
static void OpD0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Relative (reg, icpu, cpu);
    BranchCheck1 ();
    if (!CHECKZERO())
    {
	cpu->PC = cpu->PCBase + OpAddress;

#ifdef VAR_CYCLES
	cpu->Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	cpu->Cycles++;
#endif
#endif
	CPUShutdown (icpu, cpu);
    }
}

/* BPL */
static void Op10 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Relative (reg, icpu, cpu);
    BranchCheck1 ();
    if (!CHECKNEGATIVE())
    {
	cpu->PC = cpu->PCBase + OpAddress;
#ifdef VAR_CYCLES
	cpu->Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	cpu->Cycles++;
#endif
#endif
	CPUShutdown (icpu, cpu);
    }
}

/* BRA */
static void Op80 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Relative (reg, icpu, cpu);
    cpu->PC = cpu->PCBase + OpAddress;
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
    cpu->Cycles++;
#endif
#endif
    CPUShutdown (icpu, cpu);
}

/* BVC */
static void Op50 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Relative (reg, icpu, cpu);
    BranchCheck0 ();
    if (!CHECKOVERFLOW())
    {
	cpu->PC = cpu->PCBase + OpAddress;
#ifdef VAR_CYCLES
	cpu->Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	cpu->Cycles++;
#endif
#endif
	CPUShutdown (icpu, cpu);
    }
}

/* BVS */
static void Op70 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Relative (reg, icpu, cpu);
    BranchCheck0 ();
    if (CHECKOVERFLOW())
    {
	cpu->PC = cpu->PCBase + OpAddress;
#ifdef VAR_CYCLES
	cpu->Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	cpu->Cycles++;
#endif
#endif
	CPUShutdown (icpu, cpu);
    }
}
/**********************************************************************************************/

/* ClearFlag Instructions ******************************************************************** */
/* CLC */
static void Op18 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    CLEARCARRY ();
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

/* CLD */
static void OpD8 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    CLEARDECIMAL_OP ();
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

/* CLI */
static void Op58 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    CLEARIRQ ();
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
/*    CHECK_FOR_IRQ(reg, icpu, cpu); */
}

/* CLV */
static void OpB8 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    CLEAROVERFLOW();
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* DEX/DEY *********************************************************************************** */
static void OpCAX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->XL--;
    SETZN8 (reg->XL);
}

static void OpCAX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->X.W--;
    SETZN16 (reg->X.W);
}

static void Op88X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->YL--;
    SETZN8 (reg->YL);
}

static void Op88X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->Y.W--;
    SETZN16 (reg->Y.W);
}
/**********************************************************************************************/

/* INX/INY *********************************************************************************** */
static void OpE8X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->XL++;
    SETZN8 (reg->XL);
}

static void OpE8X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->X.W++;
    SETZN16 (reg->X.W);
}

static void OpC8X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->YL++;
    SETZN8 (reg->YL);
}

static void OpC8X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->Y.W++;
    SETZN16 (reg->Y.W);
}

/**********************************************************************************************/

/* NOP *************************************************************************************** */
static void OpEA (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif

}
/**********************************************************************************************/

/* PUSH Instructions ************************************************************************* */
#define PUSHW_OP(w) \
    S9xSetWord (w, reg->S.W - 1, cpu);\
    reg->S.W -= 2;
#define PUSHB_OP(b)\
    S9xSetByte (b, reg->S.W--, cpu);
#define PUSHW(w) \
    S9xSetWord (w, Registers.S.W - 1, &CPU);\
    Registers.S.W -= 2;
#define PUSHB(b)\
    S9xSetByte (b, Registers.S.W--, &CPU);
/*
#define PUSHW_OP(w) \
    S9xSetWord (w, reg->S.W - 1, cpu);\
    reg->S.W -= 2;
#define PUSHB_OP(b)\
    S9xSetByte (b, reg->S.W--, cpu);
*/
static void OpF4 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    PUSHW_OP (OpAddress);
}

static void OpD4 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = DirectIndirect (reg, icpu, cpu);
    PUSHW_OP (OpAddress);
}

static void Op62 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = RelativeLong (reg, icpu, cpu);
    PUSHW_OP (OpAddress);
}

static void Op48M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PUSHB_OP (reg->AL);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

static void Op48M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PUSHW_OP (reg->A.W);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

static void Op8B (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PUSHB_OP (reg->DB);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

static void Op0B (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PUSHW_OP (reg->D.W);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

static void Op4B (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PUSHB_OP (reg->PB);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

static void Op08 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    S9xPackStatus_OP();
    PUSHB_OP (reg->PL);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

static void OpDAX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PUSHB_OP (reg->XL);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

static void OpDAX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PUSHW_OP (reg->X.W);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

static void Op5AX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PUSHB_OP (reg->YL);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

static void Op5AX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PUSHW_OP (reg->Y.W);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* PULL Instructions ************************************************************************* */
#define PullW(w) \
	w = S9xGetWord (reg->S.W + 1, cpu); \
	reg->S.W += 2;

#define PullB(b)\
	b = S9xGetByte (++reg->S.W, cpu);

static void Op68M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
    PullB (reg->AL);
    SETZN8 (reg->AL);
}

static void Op68M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
    PullW (reg->A.W);
    SETZN16 (reg->A.W);
}

static void OpAB (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
    PullB (reg->DB);
    SETZN8 (reg->DB);
    icpu->ShiftedDB = (reg->DB & 0xff) << 16;
}

/* PHP */
static void Op2B (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
    PullW (reg->D.W);
    SETZN16 (reg->D.W);
}

/* PLP */
static void Op28 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
    PullB (reg->PL);
    S9xUnpackStatus_OP ();

    if (CHECKINDEX())
    {
	reg->XH = 0;
	reg->YH = 0;
    }
    S9xFixCycles(reg, icpu);
/*     CHECK_FOR_IRQ(reg, icpu, cpu);*/
}

static void OpFAX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
    PullB (reg->XL);
    SETZN8 (reg->XL);
}

static void OpFAX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
    PullW (reg->X.W);
    SETZN16 (reg->X.W);
}

static void Op7AX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
    PullB (reg->YL);
    SETZN8 (reg->YL);
}

static void Op7AX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
    PullW (reg->Y.W);
    SETZN16 (reg->Y.W);
}

/**********************************************************************************************/

/* SetFlag Instructions ********************************************************************** */
/* SEC */
static void Op38 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    SETCARRY();
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

/* SED */
static void OpF8 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    SETDECIMAL();
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    missing.decimal_mode = 1;
}

/* SEI */
static void Op78 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    SETIRQ_OP ();
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* Transfer Instructions ********************************************************************* */
/* TAX8 */
static void OpAAX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->XL = reg->AL;
    SETZN8 (reg->XL);
}

/* TAX16 */
static void OpAAX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->X.W = reg->A.W;
    SETZN16 (reg->X.W);
}

/* TAY8 */
static void OpA8X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->YL = reg->AL;
    SETZN8 (reg->YL);
}

/* TAY16 */
static void OpA8X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->Y.W = reg->A.W;
    SETZN16 (reg->Y.W);
}

static void Op5B (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->D.W = reg->A.W;
    SETZN16 (reg->D.W);
}

static void Op1B (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->S.W = reg->A.W;
    if (CHECKEMULATION_OP())
	reg->SH = 1;
}

static void Op7B (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->A.W = reg->D.W;
    SETZN16 (reg->A.W);
}

static void Op3B (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->A.W = reg->S.W;
    SETZN16 (reg->A.W);
}

static void OpBAX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->XL = reg->SL;
    SETZN8 (reg->XL);
}

static void OpBAX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->X.W = reg->S.W;
    SETZN16 (reg->X.W);
}

static void Op8AM1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->AL = reg->XL;
    SETZN8 (reg->AL);
}

static void Op8AM0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->A.W = reg->X.W;
    SETZN16 (reg->A.W);
}

static void Op9A (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->S.W = reg->X.W;
    if (CHECKEMULATION_OP())
	reg->SH = 1;
}

static void Op9BX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->YL = reg->XL;
    SETZN8 (reg->YL);
}

static void Op9BX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->Y.W = reg->X.W;
    SETZN16 (reg->Y.W);
}

static void Op98M1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->AL = reg->YL;
    SETZN8 (reg->AL);
}

static void Op98M0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->A.W = reg->Y.W;
    SETZN16 (reg->A.W);
}

static void OpBBX1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->XL = reg->YL;
    SETZN8 (reg->XL);
}

static void OpBBX0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    reg->X.W = reg->Y.W;
    SETZN16 (reg->X.W);
}

/**********************************************************************************************/

/* XCE *************************************************************************************** */
static void OpFB (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif

    uint8 A1 = (icpu->_Carry & 0xff);
    uint8 A2 = reg->PH;
    icpu->_Carry = A2 & 1;
    reg->PH = A1;

    if (CHECKEMULATION_OP())
    {
	SETFLAGS_OP(MemoryFlag | IndexFlag);
	reg->SH = 1;
	missing.emulate6502 = 1;
    }
    if (CHECKINDEX ())
    {
	reg->XH = 0;
	reg->YH = 0;
    }
    S9xFixCycles(reg, icpu);
}
/**********************************************************************************************/

/* BRK *************************************************************************************** */
static void Op00 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef DEBUGGER
    if (cpu->Flags & TRACE_FLAG)
	S9xTraceMessage ("*** BRK");
#endif

#ifndef SA1_OPCODES
    cpu->BRKTriggered = TRUE;
#endif

    if (!CHECKEMULATION_OP())
    {
	PUSHB_OP (reg->PB);
	PUSHW_OP (cpu->PC - cpu->PCBase + 1);
	S9xPackStatus_OP ();
	PUSHB_OP (reg->PL);
	CLEARDECIMAL_OP ();
	SETIRQ_OP ();

	reg->PB = 0;
	icpu->ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFE6, cpu), cpu);
#ifdef VAR_CYCLES
        cpu->Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	cpu->Cycles += 8;
#endif
#endif
    }
    else
    {
	PUSHW_OP (cpu->PC - cpu->PCBase);
	S9xPackStatus_OP ();
	PUSHB_OP (reg->PL);
	CLEARDECIMAL_OP ();
	SETIRQ_OP ();

	reg->PB = 0;
	icpu->ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFFE, cpu), cpu);
#ifdef VAR_CYCLES
	cpu->Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	cpu->Cycles += 6;
#endif
#endif
    }
}
/**********************************************************************************************/

/* BRL ************************************************************************************** */
static void Op82 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = RelativeLong (reg, icpu, cpu);
    S9xSetPCBase (icpu->ShiftedPB + OpAddress, cpu);
}
/**********************************************************************************************/

/* IRQ *************************************************************************************** */
void S9xOpcode_IRQ ()
{
#ifdef DEBUGGER
    if (CPU.Flags & TRACE_FLAG)
	S9xTraceMessage ("*** IRQ");
#endif
    if (!CHECKEMULATION())
    {
	PUSHB (Registers.PB);
	PUSHW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	CLEARDECIMAL ();
	SETIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2207] |
			 (Memory.FillRAM [0x2208] << 8), &SA1);
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x40))
	    S9xSetPCBase (Memory.FillRAM [0x220e] | 
			  (Memory.FillRAM [0x220f] << 8), &CPU);
	else
	    S9xSetPCBase (S9xGetWord (0xFFEE, &CPU), &CPU);
#endif
#ifdef VAR_CYCLES
        CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 8;
#endif
#endif
    }
    else
    {
	PUSHW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	CLEARDECIMAL ();
	SETIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2207] |
			 (Memory.FillRAM [0x2208] << 8), &SA1);
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x40))
	    S9xSetPCBase (Memory.FillRAM [0x220e] | 
			  (Memory.FillRAM [0x220f] << 8), &CPU);
	else
	    S9xSetPCBase (S9xGetWord (0xFFFE, &CPU), &CPU);
#endif
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 6;
#endif
#endif
    }
}

/**********************************************************************************************/

/* NMI *************************************************************************************** */
void S9xOpcode_NMI ()
{
#ifdef DEBUGGER
    if (CPU.Flags & TRACE_FLAG)
	S9xTraceMessage ("*** NMI");
#endif
    if (!CHECKEMULATION())
    {
	PUSHB (Registers.PB);
	PUSHW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	CLEARDECIMAL ();
	SETIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2205] |
			 (Memory.FillRAM [0x2206] << 8), &SA1);
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x20))
	    S9xSetPCBase (Memory.FillRAM [0x220c] |
			  (Memory.FillRAM [0x220d] << 8), &CPU);
	else
	    S9xSetPCBase (S9xGetWord (0xFFEA, &CPU), &CPU);
#endif
#ifdef VAR_CYCLES
	CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 8;
#endif
#endif
    }
    else
    {
	PUSHW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	CLEARDECIMAL ();
	SETIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2205] |
			 (Memory.FillRAM [0x2206] << 8), &SA1);
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x20))
	    S9xSetPCBase (Memory.FillRAM [0x220c] |
			  (Memory.FillRAM [0x220d] << 8), &CPU);
	else
	    S9xSetPCBase (S9xGetWord (0xFFFA, &CPU), &CPU);
#endif
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 6;
#endif
#endif
    }
}
/**********************************************************************************************/

/* COP *************************************************************************************** */
static void Op02 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef DEBUGGER
    if (cpu->Flags & TRACE_FLAG)
	S9xTraceMessage ("*** COP");
#endif	
    if (!CHECKEMULATION_OP())
    {
	PUSHB_OP (reg->PB);
	PUSHW_OP (cpu->PC - cpu->PCBase + 1);
	S9xPackStatus_OP ();
	PUSHB_OP (reg->PL);
	CLEARDECIMAL_OP ();
	SETIRQ_OP ();

	reg->PB = 0;
	icpu->ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFE4, cpu), cpu);
#ifdef VAR_CYCLES
        cpu->Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	cpu->Cycles += 8;
#endif
#endif
    }
    else
    {
	PUSHW_OP (cpu->PC - cpu->PCBase);
	S9xPackStatus_OP ();
	PUSHB_OP (reg->PL);
	CLEARDECIMAL_OP ();
	SETIRQ_OP ();

	reg->PB = 0;
	icpu->ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFF4, cpu), cpu);
#ifdef VAR_CYCLES
	cpu->Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	cpu->Cycles += 6;
#endif
#endif
    }
}
/**********************************************************************************************/

/* JML *************************************************************************************** */
static void OpDC (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndirectLong (reg, icpu, cpu);
    reg->PB = (uint8) (OpAddress >> 16);
    icpu->ShiftedPB = OpAddress & 0xff0000;
    S9xSetPCBase (OpAddress, cpu);
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
}

static void Op5C (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    reg->PB = (uint8) (OpAddress >> 16);
    icpu->ShiftedPB = OpAddress & 0xff0000;
    S9xSetPCBase (OpAddress, cpu);
}
/**********************************************************************************************/

/* JMP *************************************************************************************** */
static void Op4C (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    S9xSetPCBase (icpu->ShiftedPB + (OpAddress & 0xffff), cpu);
#if defined(CPU_SHUTDOWN) && defined(SA1_OPCODES)
    CPUShutdown (icpu, cpu);
#endif
}

static void Op6C (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndirect (reg, icpu, cpu);
    S9xSetPCBase (icpu->ShiftedPB + (OpAddress & 0xffff), cpu);
}

static void Op7C (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedIndirect (reg, icpu, cpu);
    S9xSetPCBase (icpu->ShiftedPB + OpAddress, cpu);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* JSL/RTL *********************************************************************************** */
static void Op22 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteLong (reg, icpu, cpu);
    PUSHB_OP (reg->PB);
    PUSHW_OP (cpu->PC - cpu->PCBase - 1);
    reg->PB = (uint8) (OpAddress >> 16);
    icpu->ShiftedPB = OpAddress & 0xff0000;
    S9xSetPCBase (OpAddress, cpu);
}

static void Op6B (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PullW (reg->PC);
    PullB (reg->PB);
    icpu->ShiftedPB = (reg->PB & 0xff) << 16;
    S9xSetPCBase (icpu->ShiftedPB + ((reg->PC + 1) & 0xffff), cpu);
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
}
/**********************************************************************************************/

/* JSR/RTS *********************************************************************************** */
static void Op20 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = Absolute (reg, icpu, cpu);
    PUSHW_OP (cpu->PC - cpu->PCBase - 1);
    S9xSetPCBase (icpu->ShiftedPB + (OpAddress & 0xffff), cpu);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

static void OpFC (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = AbsoluteIndexedIndirect (reg, icpu, cpu);
    PUSHW_OP (cpu->PC - cpu->PCBase - 1);
    S9xSetPCBase (icpu->ShiftedPB + OpAddress, cpu);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
}

static void Op60 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PullW (reg->PC);
    S9xSetPCBase (icpu->ShiftedPB + ((reg->PC + 1) & 0xffff), cpu);
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE * 3;
#endif
}

/**********************************************************************************************/

/* MVN/MVP *********************************************************************************** */
static void Op54X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2 + TWO_CYCLES;
#endif
    
    reg->DB = *cpu->PC++;
    icpu->ShiftedDB = (reg->DB & 0xff) << 16;
    SrcBank = *cpu->PC++;

    S9xSetByte (S9xGetByte ((SrcBank << 16) + reg->X.W, cpu), 
	     icpu->ShiftedDB + reg->Y.W, cpu);

    reg->XL++;
    reg->YL++;
    reg->A.W--;
    if (reg->A.W != 0xffff)
	cpu->PC -= 3;
}

static void Op54X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2 + TWO_CYCLES;
#endif
    
    reg->DB = *cpu->PC++;
    icpu->ShiftedDB = (reg->DB & 0xff) << 16;
    SrcBank = *cpu->PC++;

    S9xSetByte (S9xGetByte ((SrcBank << 16) + reg->X.W, cpu), 
	     icpu->ShiftedDB + reg->Y.W, cpu);

    reg->X.W++;
    reg->Y.W++;
    reg->A.W--;
    if (reg->A.W != 0xffff)
	cpu->PC -= 3;
}

static void Op44X1 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2 + TWO_CYCLES;
#endif    
    reg->DB = *cpu->PC++;
    icpu->ShiftedDB = (reg->DB & 0xff) << 16;
    SrcBank = *cpu->PC++;
    S9xSetByte (S9xGetByte ((SrcBank << 16) + reg->X.W, cpu), 
	     icpu->ShiftedDB + reg->Y.W, cpu);

    reg->XL--;
    reg->YL--;
    reg->A.W--;
    if (reg->A.W != 0xffff)
	cpu->PC -= 3;
}

static void Op44X0 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2 + TWO_CYCLES;
#endif    
    reg->DB = *cpu->PC++;
    icpu->ShiftedDB = (reg->DB & 0xff) << 16;
    SrcBank = *cpu->PC++;
    S9xSetByte (S9xGetByte ((SrcBank << 16) + reg->X.W, cpu), 
	     icpu->ShiftedDB + reg->Y.W, cpu);

    reg->X.W--;
    reg->Y.W--;
    reg->A.W--;
    if (reg->A.W != 0xffff)
	cpu->PC -= 3;
}

/**********************************************************************************************/

/* REP/SEP *********************************************************************************** */
static void OpC2 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint8 Work8 = ~*cpu->PC++;
    reg->PL &= Work8;
    icpu->_Carry &= Work8;
    icpu->_Overflow &= (Work8 >> 6);
    icpu->_Negative &= Work8;
    icpu->_Zero |= ~Work8 & Zero;

#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed + ONE_CYCLE;
#endif
    if (CHECKEMULATION_OP())
    {
	SETFLAGS_OP(MemoryFlag | IndexFlag);
	missing.emulate6502 = 1;
    }
    if (CHECKINDEX ())
    {
	reg->XH = 0;
	reg->YH = 0;
    }
    S9xFixCycles(reg, icpu);
/*    CHECK_FOR_IRQ(reg, icpu, cpu); */
}

static void OpE2 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint8 Work8 = *cpu->PC++;
    reg->PL |= Work8;
    icpu->_Carry |= Work8 & 1;
    icpu->_Overflow |= (Work8 >> 6) & 1;
    icpu->_Negative |= Work8;
    if (Work8 & Zero)
	icpu->_Zero = 0;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed + ONE_CYCLE;
#endif
    if (CHECKEMULATION_OP())
    {
	SETFLAGS_OP(MemoryFlag | IndexFlag);
	missing.emulate6502 = 1;
    }
    if (CHECKINDEX ())
    {
	reg->XH = 0;
	reg->YH = 0;
    }
    S9xFixCycles(reg, icpu);
}
/**********************************************************************************************/

/* XBA *************************************************************************************** */
static void OpEB (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint8 Work8 = reg->AL;
    reg->AL = reg->AH;
    reg->AH = Work8;

    SETZN8 (reg->AL);
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
}
/**********************************************************************************************/

/* RTI *************************************************************************************** */
static void Op40 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    PullB (reg->PL);
    S9xUnpackStatus_OP ();
    PullW (reg->PC);
    if (!CHECKEMULATION_OP())
    {
	PullB (reg->PB);
	icpu->ShiftedPB = (reg->PB & 0xff) << 16;
    }
    else
    {
	SETFLAGS_OP (MemoryFlag | IndexFlag);
	missing.emulate6502 = 1;
    }
    S9xSetPCBase (icpu->ShiftedPB + reg->PC, cpu);
    
    if (CHECKINDEX ())
    {
	reg->XH = 0;
	reg->YH = 0;
    }
#ifdef VAR_CYCLES
    cpu->Cycles += TWO_CYCLES;
#endif
    S9xFixCycles(reg, icpu);
/*    CHECK_FOR_IRQ(reg, icpu, cpu); */
}

/**********************************************************************************************/

/* STP/WAI/DB ******************************************************************************** */
// WAI
static void OpCB (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
	struct SIAPU *iapu = &IAPU;
	struct SAPU *apu = &APU;

// Ok, let's just C-ify the ASM versions separately.
#ifdef SA1_OPCODES
    SA1.WaitingForInterrupt = TRUE;
    SA1.PC--;
#if 0
// XXX: FIXME
    if(Settings.Shutdown){
        SA1.Cycles = SA1.NextEvent;
        if (IAPU.APUExecuting)
        {
            SA1.Executing = FALSE;
            do
            {
                APU_EXECUTE1 ();
            } while (APU.Cycles < SA1.NextEvent);
            SA1.Executing = TRUE;
        }
    }
#endif
#else // SA1_OPCODES
#if 0


    if (CPU.IRQActive)
    {
#ifndef SA1_OPCODES
	CPU.Cycles += TWO_CYCLES;
#endif
    }
    else
#endif
    {
		if (DSP1.version == 3) return;

	CPU.WaitingForInterrupt = TRUE;
	CPU.PC--;
#ifdef CPU_SHUTDOWN
	if (Settings.Shutdown)
	{
	    CPU.Cycles = CPU.NextEvent;
		S9xUpdateAPUTimer();
	    if (iapu->APUExecuting)
	    {
		icpu->CPUExecuting = FALSE;
		do
		{
		    APU_EXECUTE1 ();
		} while (APU.Cycles < CPU.NextEvent);
		icpu->CPUExecuting = TRUE;
	    }
	}
	else
        {
#ifndef SA1_OPCODES
            CPU.Cycles += TWO_CYCLES;
#endif
#endif
    }
    }
#endif // SA1_OPCODES
}

// STP
static void OpDB (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    cpu->PC--;
    cpu->Flags |= DEBUG_MODE_FLAG;
}

// Reserved S9xOpcode
static void Op42 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
}

/**********************************************************************************************/

/**********************************************************************************************/
/* CPU-S9xOpcodes Definitions                                                                    */
/**********************************************************************************************/
struct SOpcodes S9xOpcodesM1X1[256] =
{
    {Op00},	 {Op01M1},    {Op02},      {Op03M1},    {Op04M1},
    {Op05M1},    {Op06M1},    {Op07M1},    {Op08},      {Op09M1},
    {Op0AM1},    {Op0B},      {Op0CM1},    {Op0DM1},    {Op0EM1},
    {Op0FM1},    {Op10},      {Op11M1},    {Op12M1},    {Op13M1},
    {Op14M1},    {Op15M1},    {Op16M1},    {Op17M1},    {Op18},
    {Op19M1},    {Op1AM1},    {Op1B},      {Op1CM1},    {Op1DM1},
    {Op1EM1},    {Op1FM1},    {Op20},      {Op21M1},    {Op22},
    {Op23M1},    {Op24M1},    {Op25M1},    {Op26M1},    {Op27M1},
    {Op28},      {Op29M1},    {Op2AM1},    {Op2B},      {Op2CM1},
    {Op2DM1},    {Op2EM1},    {Op2FM1},    {Op30},      {Op31M1},
    {Op32M1},    {Op33M1},    {Op34M1},    {Op35M1},    {Op36M1},
    {Op37M1},    {Op38},      {Op39M1},    {Op3AM1},    {Op3B},
    {Op3CM1},    {Op3DM1},    {Op3EM1},    {Op3FM1},    {Op40},
    {Op41M1},    {Op42},      {Op43M1},    {Op44X1},    {Op45M1},
    {Op46M1},    {Op47M1},    {Op48M1},    {Op49M1},    {Op4AM1},
    {Op4B},      {Op4C},      {Op4DM1},    {Op4EM1},    {Op4FM1},
    {Op50},      {Op51M1},    {Op52M1},    {Op53M1},    {Op54X1},
    {Op55M1},    {Op56M1},    {Op57M1},    {Op58},      {Op59M1},
    {Op5AX1},    {Op5B},      {Op5C},      {Op5DM1},    {Op5EM1},
    {Op5FM1},    {Op60},      {Op61M1},    {Op62},      {Op63M1},
    {Op64M1},    {Op65M1},    {Op66M1},    {Op67M1},    {Op68M1},
    {Op69M1},    {Op6AM1},    {Op6B},      {Op6C},      {Op6DM1},
    {Op6EM1},    {Op6FM1},    {Op70},      {Op71M1},    {Op72M1},
    {Op73M1},    {Op74M1},    {Op75M1},    {Op76M1},    {Op77M1},
    {Op78},      {Op79M1},    {Op7AX1},    {Op7B},      {Op7C},
    {Op7DM1},    {Op7EM1},    {Op7FM1},    {Op80},      {Op81M1},
    {Op82},      {Op83M1},    {Op84X1},    {Op85M1},    {Op86X1},
    {Op87M1},    {Op88X1},    {Op89M1},    {Op8AM1},    {Op8B},
    {Op8CX1},    {Op8DM1},    {Op8EX1},    {Op8FM1},    {Op90},
    {Op91M1},    {Op92M1},    {Op93M1},    {Op94X1},    {Op95M1},
    {Op96X1},    {Op97M1},    {Op98M1},    {Op99M1},    {Op9A},
    {Op9BX1},    {Op9CM1},    {Op9DM1},    {Op9EM1},    {Op9FM1},
    {OpA0X1},    {OpA1M1},    {OpA2X1},    {OpA3M1},    {OpA4X1},
    {OpA5M1},    {OpA6X1},    {OpA7M1},    {OpA8X1},    {OpA9M1},
    {OpAAX1},    {OpAB},      {OpACX1},    {OpADM1},    {OpAEX1},
    {OpAFM1},    {OpB0},      {OpB1M1},    {OpB2M1},    {OpB3M1},
    {OpB4X1},    {OpB5M1},    {OpB6X1},    {OpB7M1},    {OpB8},
    {OpB9M1},    {OpBAX1},    {OpBBX1},    {OpBCX1},    {OpBDM1},
    {OpBEX1},    {OpBFM1},    {OpC0X1},    {OpC1M1},    {OpC2},
    {OpC3M1},    {OpC4X1},    {OpC5M1},    {OpC6M1},    {OpC7M1},
    {OpC8X1},    {OpC9M1},    {OpCAX1},    {OpCB},      {OpCCX1},
    {OpCDM1},    {OpCEM1},    {OpCFM1},    {OpD0},      {OpD1M1},
    {OpD2M1},    {OpD3M1},    {OpD4},      {OpD5M1},    {OpD6M1},
    {OpD7M1},    {OpD8},      {OpD9M1},    {OpDAX1},    {OpDB},
    {OpDC},      {OpDDM1},    {OpDEM1},    {OpDFM1},    {OpE0X1},
    {OpE1M1},    {OpE2},      {OpE3M1},    {OpE4X1},    {OpE5M1},
    {OpE6M1},    {OpE7M1},    {OpE8X1},    {OpE9M1},    {OpEA},
    {OpEB},      {OpECX1},    {OpEDM1},    {OpEEM1},    {OpEFM1},
    {OpF0},      {OpF1M1},    {OpF2M1},    {OpF3M1},    {OpF4},
    {OpF5M1},    {OpF6M1},    {OpF7M1},    {OpF8},      {OpF9M1},
    {OpFAX1},    {OpFB},      {OpFC},      {OpFDM1},    {OpFEM1},
    {OpFFM1}
};

struct SOpcodes S9xOpcodesM1X0[256] =
{
    {Op00},	 {Op01M1},    {Op02},      {Op03M1},    {Op04M1},
    {Op05M1},    {Op06M1},    {Op07M1},    {Op08},      {Op09M1},
    {Op0AM1},    {Op0B},      {Op0CM1},    {Op0DM1},    {Op0EM1},
    {Op0FM1},    {Op10},      {Op11M1},    {Op12M1},    {Op13M1},
    {Op14M1},    {Op15M1},    {Op16M1},    {Op17M1},    {Op18},
    {Op19M1},    {Op1AM1},    {Op1B},      {Op1CM1},    {Op1DM1},
    {Op1EM1},    {Op1FM1},    {Op20},      {Op21M1},    {Op22},
    {Op23M1},    {Op24M1},    {Op25M1},    {Op26M1},    {Op27M1},
    {Op28},      {Op29M1},    {Op2AM1},    {Op2B},      {Op2CM1},
    {Op2DM1},    {Op2EM1},    {Op2FM1},    {Op30},      {Op31M1},
    {Op32M1},    {Op33M1},    {Op34M1},    {Op35M1},    {Op36M1},
    {Op37M1},    {Op38},      {Op39M1},    {Op3AM1},    {Op3B},
    {Op3CM1},    {Op3DM1},    {Op3EM1},    {Op3FM1},    {Op40},
    {Op41M1},    {Op42},      {Op43M1},    {Op44X0},    {Op45M1},
    {Op46M1},    {Op47M1},    {Op48M1},    {Op49M1},    {Op4AM1},
    {Op4B},      {Op4C},      {Op4DM1},    {Op4EM1},    {Op4FM1},
    {Op50},      {Op51M1},    {Op52M1},    {Op53M1},    {Op54X0},
    {Op55M1},    {Op56M1},    {Op57M1},    {Op58},      {Op59M1},
    {Op5AX0},    {Op5B},      {Op5C},      {Op5DM1},    {Op5EM1},
    {Op5FM1},    {Op60},      {Op61M1},    {Op62},      {Op63M1},
    {Op64M1},    {Op65M1},    {Op66M1},    {Op67M1},    {Op68M1},
    {Op69M1},    {Op6AM1},    {Op6B},      {Op6C},      {Op6DM1},
    {Op6EM1},    {Op6FM1},    {Op70},      {Op71M1},    {Op72M1},
    {Op73M1},    {Op74M1},    {Op75M1},    {Op76M1},    {Op77M1},
    {Op78},      {Op79M1},    {Op7AX0},    {Op7B},      {Op7C},
    {Op7DM1},    {Op7EM1},    {Op7FM1},    {Op80},      {Op81M1},
    {Op82},      {Op83M1},    {Op84X0},    {Op85M1},    {Op86X0},
    {Op87M1},    {Op88X0},    {Op89M1},    {Op8AM1},    {Op8B},
    {Op8CX0},    {Op8DM1},    {Op8EX0},    {Op8FM1},    {Op90},
    {Op91M1},    {Op92M1},    {Op93M1},    {Op94X0},    {Op95M1},
    {Op96X0},    {Op97M1},    {Op98M1},    {Op99M1},    {Op9A},
    {Op9BX0},    {Op9CM1},    {Op9DM1},    {Op9EM1},    {Op9FM1},
    {OpA0X0},    {OpA1M1},    {OpA2X0},    {OpA3M1},    {OpA4X0},
    {OpA5M1},    {OpA6X0},    {OpA7M1},    {OpA8X0},    {OpA9M1},
    {OpAAX0},    {OpAB},      {OpACX0},    {OpADM1},    {OpAEX0},
    {OpAFM1},    {OpB0},      {OpB1M1},    {OpB2M1},    {OpB3M1},
    {OpB4X0},    {OpB5M1},    {OpB6X0},    {OpB7M1},    {OpB8},
    {OpB9M1},    {OpBAX0},    {OpBBX0},    {OpBCX0},    {OpBDM1},
    {OpBEX0},    {OpBFM1},    {OpC0X0},    {OpC1M1},    {OpC2},
    {OpC3M1},    {OpC4X0},    {OpC5M1},    {OpC6M1},    {OpC7M1},
    {OpC8X0},    {OpC9M1},    {OpCAX0},    {OpCB},      {OpCCX0},
    {OpCDM1},    {OpCEM1},    {OpCFM1},    {OpD0},      {OpD1M1},
    {OpD2M1},    {OpD3M1},    {OpD4},      {OpD5M1},    {OpD6M1},
    {OpD7M1},    {OpD8},      {OpD9M1},    {OpDAX0},    {OpDB},
    {OpDC},      {OpDDM1},    {OpDEM1},    {OpDFM1},    {OpE0X0},
    {OpE1M1},    {OpE2},      {OpE3M1},    {OpE4X0},    {OpE5M1},
    {OpE6M1},    {OpE7M1},    {OpE8X0},    {OpE9M1},    {OpEA},
    {OpEB},      {OpECX0},    {OpEDM1},    {OpEEM1},    {OpEFM1},
    {OpF0},      {OpF1M1},    {OpF2M1},    {OpF3M1},    {OpF4},
    {OpF5M1},    {OpF6M1},    {OpF7M1},    {OpF8},      {OpF9M1},
    {OpFAX0},    {OpFB},      {OpFC},      {OpFDM1},    {OpFEM1},
    {OpFFM1}
};

struct SOpcodes S9xOpcodesM0X0[256] =
{
    {Op00},	 {Op01M0},    {Op02},      {Op03M0},    {Op04M0},
    {Op05M0},    {Op06M0},    {Op07M0},    {Op08},      {Op09M0},
    {Op0AM0},    {Op0B},      {Op0CM0},    {Op0DM0},    {Op0EM0},
    {Op0FM0},    {Op10},      {Op11M0},    {Op12M0},    {Op13M0},
    {Op14M0},    {Op15M0},    {Op16M0},    {Op17M0},    {Op18},
    {Op19M0},    {Op1AM0},    {Op1B},      {Op1CM0},    {Op1DM0},
    {Op1EM0},    {Op1FM0},    {Op20},      {Op21M0},    {Op22},
    {Op23M0},    {Op24M0},    {Op25M0},    {Op26M0},    {Op27M0},
    {Op28},      {Op29M0},    {Op2AM0},    {Op2B},      {Op2CM0},
    {Op2DM0},    {Op2EM0},    {Op2FM0},    {Op30},      {Op31M0},
    {Op32M0},    {Op33M0},    {Op34M0},    {Op35M0},    {Op36M0},
    {Op37M0},    {Op38},      {Op39M0},    {Op3AM0},    {Op3B},
    {Op3CM0},    {Op3DM0},    {Op3EM0},    {Op3FM0},    {Op40},
    {Op41M0},    {Op42},      {Op43M0},    {Op44X0},    {Op45M0},
    {Op46M0},    {Op47M0},    {Op48M0},    {Op49M0},    {Op4AM0},
    {Op4B},      {Op4C},      {Op4DM0},    {Op4EM0},    {Op4FM0},
    {Op50},      {Op51M0},    {Op52M0},    {Op53M0},    {Op54X0},
    {Op55M0},    {Op56M0},    {Op57M0},    {Op58},      {Op59M0},
    {Op5AX0},    {Op5B},      {Op5C},      {Op5DM0},    {Op5EM0},
    {Op5FM0},    {Op60},      {Op61M0},    {Op62},      {Op63M0},
    {Op64M0},    {Op65M0},    {Op66M0},    {Op67M0},    {Op68M0},
    {Op69M0},    {Op6AM0},    {Op6B},      {Op6C},      {Op6DM0},
    {Op6EM0},    {Op6FM0},    {Op70},      {Op71M0},    {Op72M0},
    {Op73M0},    {Op74M0},    {Op75M0},    {Op76M0},    {Op77M0},
    {Op78},      {Op79M0},    {Op7AX0},    {Op7B},      {Op7C},
    {Op7DM0},    {Op7EM0},    {Op7FM0},    {Op80},      {Op81M0},
    {Op82},      {Op83M0},    {Op84X0},    {Op85M0},    {Op86X0},
    {Op87M0},    {Op88X0},    {Op89M0},    {Op8AM0},    {Op8B},
    {Op8CX0},    {Op8DM0},    {Op8EX0},    {Op8FM0},    {Op90},
    {Op91M0},    {Op92M0},    {Op93M0},    {Op94X0},    {Op95M0},
    {Op96X0},    {Op97M0},    {Op98M0},    {Op99M0},    {Op9A},
    {Op9BX0},    {Op9CM0},    {Op9DM0},    {Op9EM0},    {Op9FM0},
    {OpA0X0},    {OpA1M0},    {OpA2X0},    {OpA3M0},    {OpA4X0},
    {OpA5M0},    {OpA6X0},    {OpA7M0},    {OpA8X0},    {OpA9M0},
    {OpAAX0},    {OpAB},      {OpACX0},    {OpADM0},    {OpAEX0},
    {OpAFM0},    {OpB0},      {OpB1M0},    {OpB2M0},    {OpB3M0},
    {OpB4X0},    {OpB5M0},    {OpB6X0},    {OpB7M0},    {OpB8},
    {OpB9M0},    {OpBAX0},    {OpBBX0},    {OpBCX0},    {OpBDM0},
    {OpBEX0},    {OpBFM0},    {OpC0X0},    {OpC1M0},    {OpC2},
    {OpC3M0},    {OpC4X0},    {OpC5M0},    {OpC6M0},    {OpC7M0},
    {OpC8X0},    {OpC9M0},    {OpCAX0},    {OpCB},      {OpCCX0},
    {OpCDM0},    {OpCEM0},    {OpCFM0},    {OpD0},      {OpD1M0},
    {OpD2M0},    {OpD3M0},    {OpD4},      {OpD5M0},    {OpD6M0},
    {OpD7M0},    {OpD8},      {OpD9M0},    {OpDAX0},    {OpDB},
    {OpDC},      {OpDDM0},    {OpDEM0},    {OpDFM0},    {OpE0X0},
    {OpE1M0},    {OpE2},      {OpE3M0},    {OpE4X0},    {OpE5M0},
    {OpE6M0},    {OpE7M0},    {OpE8X0},    {OpE9M0},    {OpEA},
    {OpEB},      {OpECX0},    {OpEDM0},    {OpEEM0},    {OpEFM0},
    {OpF0},      {OpF1M0},    {OpF2M0},    {OpF3M0},    {OpF4},
    {OpF5M0},    {OpF6M0},    {OpF7M0},    {OpF8},      {OpF9M0},
    {OpFAX0},    {OpFB},      {OpFC},      {OpFDM0},    {OpFEM0},
    {OpFFM0}
};

struct SOpcodes S9xOpcodesM0X1[256] =
{
    {Op00},	 {Op01M0},    {Op02},      {Op03M0},    {Op04M0},
    {Op05M0},    {Op06M0},    {Op07M0},    {Op08},      {Op09M0},
    {Op0AM0},    {Op0B},      {Op0CM0},    {Op0DM0},    {Op0EM0},
    {Op0FM0},    {Op10},      {Op11M0},    {Op12M0},    {Op13M0},
    {Op14M0},    {Op15M0},    {Op16M0},    {Op17M0},    {Op18},
    {Op19M0},    {Op1AM0},    {Op1B},      {Op1CM0},    {Op1DM0},
    {Op1EM0},    {Op1FM0},    {Op20},      {Op21M0},    {Op22},
    {Op23M0},    {Op24M0},    {Op25M0},    {Op26M0},    {Op27M0},
    {Op28},      {Op29M0},    {Op2AM0},    {Op2B},      {Op2CM0},
    {Op2DM0},    {Op2EM0},    {Op2FM0},    {Op30},      {Op31M0},
    {Op32M0},    {Op33M0},    {Op34M0},    {Op35M0},    {Op36M0},
    {Op37M0},    {Op38},      {Op39M0},    {Op3AM0},    {Op3B},
    {Op3CM0},    {Op3DM0},    {Op3EM0},    {Op3FM0},    {Op40},
    {Op41M0},    {Op42},      {Op43M0},    {Op44X1},    {Op45M0},
    {Op46M0},    {Op47M0},    {Op48M0},    {Op49M0},    {Op4AM0},
    {Op4B},      {Op4C},      {Op4DM0},    {Op4EM0},    {Op4FM0},
    {Op50},      {Op51M0},    {Op52M0},    {Op53M0},    {Op54X1},
    {Op55M0},    {Op56M0},    {Op57M0},    {Op58},      {Op59M0},
    {Op5AX1},    {Op5B},      {Op5C},      {Op5DM0},    {Op5EM0},
    {Op5FM0},    {Op60},      {Op61M0},    {Op62},      {Op63M0},
    {Op64M0},    {Op65M0},    {Op66M0},    {Op67M0},    {Op68M0},
    {Op69M0},    {Op6AM0},    {Op6B},      {Op6C},      {Op6DM0},
    {Op6EM0},    {Op6FM0},    {Op70},      {Op71M0},    {Op72M0},
    {Op73M0},    {Op74M0},    {Op75M0},    {Op76M0},    {Op77M0},
    {Op78},      {Op79M0},    {Op7AX1},    {Op7B},      {Op7C},
    {Op7DM0},    {Op7EM0},    {Op7FM0},    {Op80},      {Op81M0},
    {Op82},      {Op83M0},    {Op84X1},    {Op85M0},    {Op86X1},
    {Op87M0},    {Op88X1},    {Op89M0},    {Op8AM0},    {Op8B},
    {Op8CX1},    {Op8DM0},    {Op8EX1},    {Op8FM0},    {Op90},
    {Op91M0},    {Op92M0},    {Op93M0},    {Op94X1},    {Op95M0},
    {Op96X1},    {Op97M0},    {Op98M0},    {Op99M0},    {Op9A},
    {Op9BX1},    {Op9CM0},    {Op9DM0},    {Op9EM0},    {Op9FM0},
    {OpA0X1},    {OpA1M0},    {OpA2X1},    {OpA3M0},    {OpA4X1},
    {OpA5M0},    {OpA6X1},    {OpA7M0},    {OpA8X1},    {OpA9M0},
    {OpAAX1},    {OpAB},      {OpACX1},    {OpADM0},    {OpAEX1},
    {OpAFM0},    {OpB0},      {OpB1M0},    {OpB2M0},    {OpB3M0},
    {OpB4X1},    {OpB5M0},    {OpB6X1},    {OpB7M0},    {OpB8},
    {OpB9M0},    {OpBAX1},    {OpBBX1},    {OpBCX1},    {OpBDM0},
    {OpBEX1},    {OpBFM0},    {OpC0X1},    {OpC1M0},    {OpC2},
    {OpC3M0},    {OpC4X1},    {OpC5M0},    {OpC6M0},    {OpC7M0},
    {OpC8X1},    {OpC9M0},    {OpCAX1},    {OpCB},      {OpCCX1},
    {OpCDM0},    {OpCEM0},    {OpCFM0},    {OpD0},      {OpD1M0},
    {OpD2M0},    {OpD3M0},    {OpD4},      {OpD5M0},    {OpD6M0},
    {OpD7M0},    {OpD8},      {OpD9M0},    {OpDAX1},    {OpDB},
    {OpDC},      {OpDDM0},    {OpDEM0},    {OpDFM0},    {OpE0X1},
    {OpE1M0},    {OpE2},      {OpE3M0},    {OpE4X1},    {OpE5M0},
    {OpE6M0},    {OpE7M0},    {OpE8X1},    {OpE9M0},    {OpEA},
    {OpEB},      {OpECX1},    {OpEDM0},    {OpEEM0},    {OpEFM0},
    {OpF0},      {OpF1M0},    {OpF2M0},    {OpF3M0},    {OpF4},
    {OpF5M0},    {OpF6M0},    {OpF7M0},    {OpF8},      {OpF9M0},
    {OpFAX1},    {OpFB},      {OpFC},      {OpFDM0},    {OpFEM0},
    {OpFFM0}
};
