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
#ifndef _65c816_h_
#define _65c816_h_

#define AL A.B.l
#define AH A.B.h
#define XL X.B.l
#define XH X.B.h
#define YL Y.B.l
#define YH Y.B.h
#define SL S.B.l
#define SH S.B.h
#define DL D.B.l
#define DH D.B.h
#define PL P.B.l
#define PH P.B.h

#define Carry       1
#define Zero        2
#define IRQ         4
#define Decimal     8
#define IndexFlag  16
#define MemoryFlag 32
#define Overflow   64
#define Negative  128
#define Emulation 256

#define CLEARCARRY()	(icpu->_Carry = 0)
#define SETCARRY()		(icpu->_Carry = 1)
#define SETZERO()		(icpu->_Zero = 0)
#define	CLEARZERO()		(icpu->_Zero = 1)
#define	SETIRQ_OP()		(reg->P.W |= IRQ)
#define SETIRQ()		(Registers.P.W |= IRQ)
#define CLEARIRQ()		(reg->P.W &= ~IRQ)
#define SETDECIMAL()	(reg->P.W |= Decimal)
#define CLEARDECIMAL()	(Registers.P.W &= ~Decimal)
#define CLEARDECIMAL_OP()	(reg->P.W &= ~Decimal)
#define SETINDEX() (	(reg->P.W |= IndexFlag)
#define CLEARINDEX()	(reg->P.W &= ~IndexFlag)
#define SETMEMORY()		(reg->P.W |= MemoryFlag)
#define CLEARMEMORY()	(reg->P.W &= ~MemoryFlag)
#define SETOVERFLOW()	(icpu->_Overflow = 1)
#define CLEAROVERFLOW() (icpu->_Overflow = 0)
#define SETNEGATIVE()	(icpu->_Negative = 0x80)
#define CLEARNEGATIVE() (icpu->_Negative = 0)

#define CHECKZERO() (icpu->_Zero == 0)
#define CHECKCARRY() (icpu->_Carry)
#define CHECKIRQ() (reg->P.W & IRQ)
#define CHECKDECIMAL() (reg->P.W & Decimal)
#define CHECKINDEX() (reg->P.W & IndexFlag)
#define CHECKMEMORY() (reg->P.W & MemoryFlag)
#define CHECKOVERFLOW() (icpu->_Overflow)
#define CHECKNEGATIVE() (icpu->_Negative & 0x80)
#define CHECKEMULATION() (Registers.P.W & Emulation)
#define CHECKEMULATION_OP() (reg->P.W & Emulation)

#define CLEARFLAGS(f) (Registers.P.W &= ~(f))
#define SETFLAGS(f)   (Registers.P.W |=  (f))
#define SETFLAGS_OP(f)   (reg->P.W |=  (f))
#define CHECKFLAG(f)  (reg->PL & (f))

typedef union
{
#ifdef LSB_FIRST
    struct { uint8 l,h; } B;
#else
    struct { uint8 h,l; } B;
#endif
    uint16 W;
} pair;

struct SRegisters{
    uint16_32	PC;
    uint8_32	PB;
    uint8_32	DB;
    pair		P;
    pair		A;
    pair		D;
    pair		S;
    pair		X;
    pair		Y;
};

/*
struct SRegisters{
    uint8  PB;
    uint8  DB;
    pair   P;
    pair   A;
    pair   D;
    pair   S;
    pair   X;
    pair   Y;
    uint16 PC;
};
*/
struct SICPU
{
    uint8		*Speed;
    struct		SOpcodes *S9xOpcodes;
    uint8_32	_Carry;
    uint8_32	_Zero;
    uint8_32	_Negative;
    uint8_32	_Overflow;
    bool8_32	CPUExecuting;
    uint32		ShiftedPB;
    uint32		ShiftedDB;
    uint32		Frame;
    uint32		Scanline;
    uint32		FrameAdvanceCount;
};

EXTERN_C struct SRegisters Registers;

#endif
