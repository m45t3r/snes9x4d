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
#ifndef _apumemory_h_
#define _apumemory_h_

#include "port.h"

START_EXTERN_C
extern uint8 W4;
extern uint8 APUROM[64];
END_EXTERN_C

INLINE uint8 S9xAPUGetByteZ (uint8 Address, struct SIAPU * iapu)
{
    if (Address >= 0xf0 && iapu->DirectPage == iapu->RAM)
    {
	if (Address >= 0xf4 && Address <= 0xf7)
	{
#ifdef SPC700_SHUTDOWN
	    iapu->WaitAddress2 = iapu->WaitAddress1;
	    iapu->WaitAddress1 = iapu->PC;
#endif	    
	    return (iapu->RAM [Address]);
	}
	if (Address >= 0xfd)
	{
#ifdef SPC700_SHUTDOWN
	    iapu->WaitAddress2 = iapu->WaitAddress1;
	    iapu->WaitAddress1 = iapu->PC;
#endif	    
	    uint8 t = iapu->RAM [Address];
	    iapu->RAM [Address] = 0;
	    return (t);
	}
	else
	if (Address == 0xf3)
	    return (S9xGetAPUDSP ());

	return (iapu->RAM [Address]);
    }
    else
	return (iapu->DirectPage [Address]);
}

INLINE void S9xAPUSetByteZ (uint8 byte, uint8 Address, struct SIAPU * iapu, struct SAPU * apu)
{
    if (Address >= 0xf0 && iapu->DirectPage == iapu->RAM)
    {
	if (Address == 0xf3)
	    S9xSetAPUDSP (byte, apu, iapu);
	else
	if (Address >= 0xf4 && Address <= 0xf7)
	    apu->OutPorts [Address - 0xf4] = byte;
	else
	if (Address == 0xf1)
	    S9xSetAPUControl (byte);
	else
	if (Address < 0xfd)
	{
	    iapu->RAM [Address] = byte;
	    if (Address >= 0xfa)
	    {
		if (byte == 0)
		    apu->TimerTarget [Address - 0xfa] = 0x100;
		else
		    apu->TimerTarget [Address - 0xfa] = byte;
	    }
	}
    }
    else
	iapu->DirectPage [Address] = byte;
}

INLINE uint8 S9xAPUGetByte (uint32 Address, struct SIAPU * iapu)
{
    Address &= 0xffff;
    
    if (Address <= 0xff && Address >= 0xf0)
    {
	if (Address >= 0xf4 && Address <= 0xf7)
	{
#ifdef SPC700_SHUTDOWN
	    iapu->WaitAddress2 = iapu->WaitAddress1;
	    iapu->WaitAddress1 = iapu->PC;
#endif	    
	    return (iapu->RAM [Address]);
	}
	else
	if (Address == 0xf3)
	    return (S9xGetAPUDSP ());
	if (Address >= 0xfd)
	{
#ifdef SPC700_SHUTDOWN
	    iapu->WaitAddress2 = iapu->WaitAddress1;
	    iapu->WaitAddress1 = iapu->PC;
#endif
	    uint8 t = iapu->RAM [Address];
	    iapu->RAM [Address] = 0;
	    return (t);
	}
	return (iapu->RAM [Address]);
    }
    else
	return (iapu->RAM [Address]);
}

INLINE void S9xAPUSetByte (uint8 byte, uint32 Address, struct SIAPU * iapu, struct SAPU * apu)
{
    Address &= 0xffff;
    
    if (Address <= 0xff && Address >= 0xf0)
    {
	if (Address == 0xf3)
	    S9xSetAPUDSP (byte, apu, iapu);
	else
	if (Address >= 0xf4 && Address <= 0xf7)
	    apu->OutPorts [Address - 0xf4] = byte;
	else
	if (Address == 0xf1)
	    S9xSetAPUControl (byte);
	else
	if (Address < 0xfd)
	{
	    iapu->RAM [Address] = byte;
	    if (Address >= 0xfa)
	    {
		if (byte == 0)
		    apu->TimerTarget [Address - 0xfa] = 0x100;
		else
		    apu->TimerTarget [Address - 0xfa] = byte;
	    }
	}
    }
    else
    {
#if 0
if (Address >= 0x2500 && Address <= 0x2504)
printf ("%06d %04x <- %02x\n", ICPU.Scanline, Address, byte);
if (Address == 0x26c6)
{
    extern FILE *apu_trace;
    extern FILE *trace;
    apu->Flags |= TRACE_FLAG;
    CPU.Flags |= TRACE_FLAG;
    if (apu_trace == NULL)
	apu_trace = fopen ("aputrace.log", "wb");
    if (trace == NULL)
	trace = fopen ("trace.log", "wb");
    printf ("TRACING SWITCHED ON\n");
}
#endif
	if (Address < 0xffc0)
	    iapu->RAM [Address] = byte;
	else
	{
	    apu->ExtraRAM [Address - 0xffc0] = byte;
	    if (!apu->ShowROM)
		iapu->RAM [Address] = byte;
	}
    }
}
#endif
