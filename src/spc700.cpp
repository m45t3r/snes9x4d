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
#include "snes9x.h"
#include "spc700.h"
#include "memmap.h"
#include "display.h"
#include "cpuexec.h"
#include "apu.h"

// SPC700/Sound DSP chips have a 24.57MHz crystal on their PCB.

#ifdef NO_INLINE_SET_GET
uint8 S9xAPUGetByteZ(uint8 address, struct SIAPU *);
uint8 S9xAPUGetByte(uint32 address, struct SIAPU *);
void S9xAPUSetByteZ(uint8, uint8 address, struct SIAPU *, struct SAPU *);
void S9xAPUSetByte(uint8, uint32 address, struct SIAPU *, struct SAPU *);

#else
#undef INLINE
#ifdef SPC700_ASM
#define INLINE extern "C"
#else
#define INLINE inline
#endif
#include "apumem.h"
#endif

START_EXTERN_C
/*
extern uint8 Work8;
extern uint16 Work16;
extern uint32 Work32;
extern signed char Int8;
extern short Int16;
extern long Int32;
extern short Int16;
extern uint8 W1;
extern uint8 W2;
*/
END_EXTERN_C

#define OP1 (*(iapu->PC + 1))
#define OP2 (*(iapu->PC + 2))

#ifdef SPC700_SHUTDOWN
#define APUShutdown()                                                          \
	if (Settings.Shutdown && (iapu->PC == iapu->WaitAddress1 ||            \
				  iapu->PC == iapu->WaitAddress2)) {           \
		if (iapu->WaitCounter == 0) {                                  \
			if (!ICPU.CPUExecuting)                                \
				apu->Cycles = CPU.Cycles = CPU.NextEvent;      \
			else                                                   \
				iapu->APUExecuting = FALSE;                    \
		} else if (iapu->WaitCounter >= 2)                             \
			iapu->WaitCounter = 1;                                 \
		else                                                           \
			iapu->WaitCounter--;                                   \
	}
#else
#define APUShutdown()
#endif

#define APUSetZN8(b) iapu->_Zero = (b);

#define APUSetZN16(w) iapu->_Zero = ((w) != 0) | ((w) >> 8);
#ifndef _ZAURUS
void STOP(char *s)
{
	char buffer[100];

#ifdef DEBUGGER
	S9xAPUOPrint(buffer, IAPU.PC - IAPU.RAM);
#endif

	sprintf(String, "Sound CPU in unknown state executing %s at %04X\n%s\n",
		s, IAPU.PC - IAPU.RAM, buffer);
	S9xMessage(S9X_ERROR, S9X_APU_STOPPED, String);
	APU.TimerEnabled[0] = APU.TimerEnabled[1] = APU.TimerEnabled[2] = FALSE;
	IAPU.APUExecuting = FALSE;

#ifdef DEBUGGER
	CPU.Flags |= DEBUG_MODE_FLAG;
#else
	S9xExit();
#endif
}
#endif
#define TCALL(n)                                                               \
	{                                                                      \
		PushW(iapu->PC - iapu->RAM + 1);                               \
		iapu->PC =                                                     \
		    iapu->RAM + (apu->ExtraRAM[((15 - n) << 1)] +              \
				 (apu->ExtraRAM[((15 - n) << 1) + 1] << 8));   \
	}

// XXX: HalfCarry
#define SBC(a, b)                                                              \
	int16 Int16 = (short)(a) - (short)(b) + (short)(APUCheckCarry()) - 1;  \
	APUClearHalfCarry();                                                   \
	iapu->_Carry = Int16 >= 0;                                             \
	if ((((a) ^ (b)) & 0x80) && (((a) ^ (uint8)Int16) & 0x80))             \
		APUSetOverflow();                                              \
	else                                                                   \
		APUClearOverflow();                                            \
	(a) = (uint8)Int16;                                                    \
	APUSetZN8((uint8)Int16);

// XXX: HalfCarry
#define ADC(a, b)                                                              \
	uint16 Work16 = (a) + (b) + APUCheckCarry();                           \
	APUClearHalfCarry();                                                   \
	iapu->_Carry = Work16 >= 0x100;                                        \
	if (~((a) ^ (b)) & ((b) ^ (uint8)Work16) & 0x80)                       \
		APUSetOverflow();                                              \
	else                                                                   \
		APUClearOverflow();                                            \
	(a) = (uint8)Work16;                                                   \
	APUSetZN8((uint8)Work16);

#define CMP(a, b)                                                              \
	int16 Int16 = (short)(a) - (short)(b);                                 \
	iapu->_Carry = Int16 >= 0;                                             \
	APUSetZN8((uint8)Int16);

#define ASL(b)                                                                 \
	iapu->_Carry = ((b)&0x80) != 0;                                        \
	(b) <<= 1;                                                             \
	APUSetZN8(b);
#define LSR(b)                                                                 \
	iapu->_Carry = (b)&1;                                                  \
	(b) >>= 1;                                                             \
	APUSetZN8(b);
#define ROL(b)                                                                 \
	uint16 Work16 = ((b) << 1) | APUCheckCarry();                          \
	iapu->_Carry = Work16 >= 0x100;                                        \
	(b) = (uint8)Work16;                                                   \
	APUSetZN8(b);
#define ROR(b)                                                                 \
	uint16 Work16 = (b) | ((uint16)APUCheckCarry() << 8);                  \
	iapu->_Carry = (uint8)Work16 & 1;                                      \
	Work16 >>= 1;                                                          \
	(b) = (uint8)Work16;                                                   \
	APUSetZN8(b);

#define Push(b)                                                                \
	*(iapu->RAM + 0x100 + areg->S) = b;                                    \
	areg->S = ((areg->S - 1) & 0xff);

#define Pop(b)                                                                 \
	areg->S = ((areg->S + 1) & 0xff);                                      \
	(b) = *(iapu->RAM + 0x100 + areg->S);

#ifdef FAST_LSB_WORD_ACCESS
#define PushW(w)                                                               \
	*(uint16 *)(iapu->RAM + 0xff + areg->S) = w;                           \
	areg->S = ((areg->S - 2) & 0xff);                                      \
	;
#define PopW(w)                                                                \
	areg->S = ((areg->S + 2) & 0xff);                                      \
	w = *(uint16 *)(iapu->RAM + 0xff + areg->S);
#else
#define PushW(w)                                                               \
	*(iapu->RAM + 0xff + areg->S) = w;                                     \
	*(iapu->RAM + 0x100 + areg->S) = (w >> 8);                             \
	areg->S = ((areg->S - 2) & 0xff);                                      \
	;
#define PopW(w)                                                                \
	areg->S = ((areg->S + 2) & 0xff);                                      \
	;                                                                      \
	(w) = *(iapu->RAM + 0xff + areg->S) +                                  \
	      (*(iapu->RAM + 0x100 + areg->S) << 8);
#endif

#define Relative()                                                             \
	int8 Int8 = OP1;                                                       \
	int16 Int16 = (int)(iapu->PC + 2 - iapu->RAM) + Int8;

#define Relative2()                                                            \
	int8 Int8 = OP2;                                                       \
	int16 Int16 = (int)(iapu->PC + 3 - iapu->RAM) + Int8;

#ifdef FAST_LSB_WORD_ACCESS
#define IndexedXIndirect()                                                     \
	iapu->Address =                                                        \
	    *(uint16 *)(iapu->DirectPage + ((OP1 + areg->X) & 0xff));

#define Absolute() iapu->Address = *(uint16 *)(iapu->PC + 1);

#define AbsoluteX() iapu->Address = *(uint16 *)(iapu->PC + 1) + areg->X;

#define AbsoluteY() iapu->Address = *(uint16 *)(iapu->PC + 1) + areg->YA.B.Y;

#define MemBit()                                                               \
	iapu->Address = *(uint16 *)(iapu->PC + 1);                             \
	iapu->Bit = (uint8)(iapu->Address >> 13);                              \
	iapu->Address &= 0x1fff;

#define IndirectIndexedY()                                                     \
	iapu->Address = *(uint16 *)(iapu->DirectPage + OP1) + areg->YA.B.Y;
#else
#define IndexedXIndirect()                                                     \
	iapu->Address =                                                        \
	    *(iapu->DirectPage + ((OP1 + areg->X) & 0xff)) +                   \
	    (*(iapu->DirectPage + ((OP1 + areg->X + 1) & 0xff)) << 8);
#define Absolute() iapu->Address = OP1 + (OP2 << 8);

#define AbsoluteX() iapu->Address = OP1 + (OP2 << 8) + areg->X;

#define AbsoluteY() iapu->Address = OP1 + (OP2 << 8) + areg->YA.B.Y;

#define MemBit()                                                               \
	iapu->Address = OP1 + (OP2 << 8);                                      \
	iapu->Bit = (int8)(iapu->Address >> 13);                               \
	iapu->Address &= 0x1fff;

#define IndirectIndexedY()                                                     \
	iapu->Address = *(iapu->DirectPage + OP1) +                            \
			(*(iapu->DirectPage + OP1 + 1) << 8) + areg->YA.B.Y;
#endif

void Apu00(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// NOP
	iapu->PC++;
}

void Apu01(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(0)
}

void Apu11(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(1)
}

void Apu21(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(2)
}

void Apu31(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(3)
}

void Apu41(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(4)
}

void Apu51(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(5)
}

void Apu61(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(6)
}

void Apu71(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(7)
}

void Apu81(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(8)
}

void Apu91(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(9)
}

void ApuA1(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(10)
}

void ApuB1(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(11)
}

void ApuC1(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(12)
}

void ApuD1(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(13)
}

void ApuE1(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(14)
}

void ApuF1(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	TCALL(15)
}

void Apu3F(struct SAPURegisters *areg, struct SIAPU *iapu,
	   struct SAPU *apu) // CALL absolute
{
	Absolute();
	// 0xB6f for Star Fox 2
	PushW(iapu->PC + 3 - iapu->RAM);
	iapu->PC = iapu->RAM + iapu->Address;
}

void Apu4F(struct SAPURegisters *areg, struct SIAPU *iapu,
	   struct SAPU *apu) // PCALL $XX
{
	uint8 Work8 = OP1;
	PushW(iapu->PC + 2 - iapu->RAM);
	iapu->PC = iapu->RAM + 0xff00 + Work8;
}

#define SET(b)                                                                 \
	S9xAPUSetByteZ((uint8)(S9xAPUGetByteZ(OP1, iapu) | (1 << (b))), OP1,   \
		       iapu, apu);                                             \
	iapu->PC += 2

void Apu02(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	SET(0);
}

void Apu22(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	SET(1);
}

void Apu42(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	SET(2);
}

void Apu62(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	SET(3);
}

void Apu82(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	SET(4);
}

void ApuA2(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	SET(5);
}

void ApuC2(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	SET(6);
}

void ApuE2(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	SET(7);
}

#define CLR(b)                                                                 \
	S9xAPUSetByteZ((uint8)(S9xAPUGetByteZ(OP1, iapu) & ~(1 << (b))), OP1,  \
		       iapu, apu);                                             \
	iapu->PC += 2;

void Apu12(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	CLR(0);
}

void Apu32(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	CLR(1);
}

void Apu52(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	CLR(2);
}

void Apu72(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	CLR(3);
}

void Apu92(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	CLR(4);
}

void ApuB2(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	CLR(5);
}

void ApuD2(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	CLR(6);
}

void ApuF2(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	CLR(7);
}

#define BBS(b)                                                                 \
	uint8 Work8 = OP1;                                                     \
	Relative2();                                                           \
	if (S9xAPUGetByteZ(Work8, iapu) & (1 << (b))) {                        \
		iapu->PC = iapu->RAM + (uint16)Int16;                          \
		apu->Cycles += iapu->TwoCycles;                                \
	} else                                                                 \
		iapu->PC += 3

void Apu03(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBS(0);
}

void Apu23(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBS(1);
}

void Apu43(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBS(2);
}

void Apu63(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBS(3);
}

void Apu83(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBS(4);
}

void ApuA3(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBS(5);
}

void ApuC3(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBS(6);
}

void ApuE3(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBS(7);
}

#define BBC(b)                                                                 \
	uint8 Work8 = OP1;                                                     \
	Relative2();                                                           \
	if (!(S9xAPUGetByteZ(Work8, iapu) & (1 << (b)))) {                     \
		iapu->PC = iapu->RAM + (uint16)Int16;                          \
		apu->Cycles += iapu->TwoCycles;                                \
	} else                                                                 \
		iapu->PC += 3

void Apu13(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBC(0);
}

void Apu33(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBC(1);
}

void Apu53(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBC(2);
}

void Apu73(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBC(3);
}

void Apu93(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBC(4);
}

void ApuB3(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBC(5);
}

void ApuD3(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBC(6);
}

void ApuF3(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	BBC(7);
}

void Apu04(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR A,dp
	areg->YA.B.A |= S9xAPUGetByteZ(OP1, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu05(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR A,abs
	Absolute();
	areg->YA.B.A |= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void Apu06(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR A,(X)
	areg->YA.B.A |= S9xAPUGetByteZ(areg->X, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC++;
}

void Apu07(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR A,(dp+X)
	IndexedXIndirect();
	areg->YA.B.A |= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu08(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR A,#00
	areg->YA.B.A |= OP1;
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu09(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR dp(dest),dp(src)
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	Work8 |= S9xAPUGetByteZ(OP2, iapu);
	S9xAPUSetByteZ(Work8, OP2, iapu, apu);
	APUSetZN8(Work8);
	iapu->PC += 3;
}

void Apu14(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR A,dp+X
	areg->YA.B.A |= S9xAPUGetByteZ(OP1 + areg->X, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu15(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR A,abs+X
	AbsoluteX();
	areg->YA.B.A |= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void Apu16(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR A,abs+Y
	AbsoluteY();
	areg->YA.B.A |= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void Apu17(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR A,(dp)+Y
	IndirectIndexedY();
	areg->YA.B.A |= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu18(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR dp,#00
	uint8 Work8 = OP1;
	Work8 |= S9xAPUGetByteZ(OP2, iapu);
	S9xAPUSetByteZ(Work8, OP2, iapu, apu);
	APUSetZN8(Work8);
	iapu->PC += 3;
}

void Apu19(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR (X),(Y)
	uint8 Work8 =
	    S9xAPUGetByteZ(areg->X, iapu) | S9xAPUGetByteZ(areg->YA.B.Y, iapu);
	APUSetZN8(Work8);
	S9xAPUSetByteZ(Work8, areg->X, iapu, apu);
	iapu->PC++;
}

void Apu0A(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR1 C,membit
	MemBit();
	if (!APUCheckCarry()) {
		if (S9xAPUGetByte(iapu->Address, iapu) & (1 << iapu->Bit))
			APUSetCarry();
	}
	iapu->PC += 3;
}

void Apu2A(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// OR1 C,not membit
	MemBit();
	if (!APUCheckCarry()) {
		if (!(S9xAPUGetByte(iapu->Address, iapu) & (1 << iapu->Bit)))
			APUSetCarry();
	}
	iapu->PC += 3;
}

void Apu4A(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND1 C,membit
	MemBit();
	if (APUCheckCarry()) {
		if (!(S9xAPUGetByte(iapu->Address, iapu) & (1 << iapu->Bit)))
			APUClearCarry();
	}
	iapu->PC += 3;
}

void Apu6A(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND1 C, not membit
	MemBit();
	if (APUCheckCarry()) {
		if ((S9xAPUGetByte(iapu->Address, iapu) & (1 << iapu->Bit)))
			APUClearCarry();
	}
	iapu->PC += 3;
}

void Apu8A(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR1 C, membit
	MemBit();
	if (APUCheckCarry()) {
		if (S9xAPUGetByte(iapu->Address, iapu) & (1 << iapu->Bit))
			APUClearCarry();
	} else {
		if (S9xAPUGetByte(iapu->Address, iapu) & (1 << iapu->Bit))
			APUSetCarry();
	}
	iapu->PC += 3;
}

void ApuAA(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV1 C,membit
	MemBit();
	if (S9xAPUGetByte(iapu->Address, iapu) & (1 << iapu->Bit))
		APUSetCarry();
	else
		APUClearCarry();
	iapu->PC += 3;
}

void ApuCA(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV1 membit,C
	MemBit();
	if (APUCheckCarry()) {
		S9xAPUSetByte(S9xAPUGetByte(iapu->Address, iapu) |
				  (1 << iapu->Bit),
			      iapu->Address, iapu, apu);
	} else {
		S9xAPUSetByte(S9xAPUGetByte(iapu->Address, iapu) &
				  ~(1 << iapu->Bit),
			      iapu->Address, iapu, apu);
	}
	iapu->PC += 3;
}

void ApuEA(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// NOT1 membit
	MemBit();
	S9xAPUSetByte(S9xAPUGetByte(iapu->Address, iapu) ^ (1 << iapu->Bit),
		      iapu->Address, iapu, apu);
	iapu->PC += 3;
}

void Apu0B(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ASL dp
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	ASL(Work8);
	S9xAPUSetByteZ(Work8, OP1, iapu, apu);
	iapu->PC += 2;
}

void Apu0C(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ASL abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	ASL(Work8);
	S9xAPUSetByte(Work8, iapu->Address, iapu, apu);
	iapu->PC += 3;
}

void Apu1B(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ASL dp+X
	uint8 Work8 = S9xAPUGetByteZ(OP1 + areg->X, iapu);
	ASL(Work8);
	S9xAPUSetByteZ(Work8, OP1 + areg->X, iapu, apu);
	iapu->PC += 2;
}

void Apu1C(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ASL A
	ASL(areg->YA.B.A);
	iapu->PC++;
}

void Apu0D(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// PUSH PSW
	S9xAPUPackStatus_OP();
	Push(areg->P);
	iapu->PC++;
}

void Apu2D(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// PUSH A
	Push(areg->YA.B.A);
	iapu->PC++;
}

void Apu4D(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// PUSH X
	Push(areg->X);
	iapu->PC++;
}

void Apu6D(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// PUSH Y
	Push(areg->YA.B.Y);
	iapu->PC++;
}

void Apu8E(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// POP PSW
	Pop(areg->P);
	S9xAPUUnpackStatus_OP();
	if (APUCHECKDIRECTPAGE_OP())
		iapu->DirectPage = iapu->RAM + 0x100;
	else
		iapu->DirectPage = iapu->RAM;
	iapu->PC++;
}

void ApuAE(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// POP A
	Pop(areg->YA.B.A);
	iapu->PC++;
}

void ApuCE(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// POP X
	Pop(areg->X);
	iapu->PC++;
}

void ApuEE(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// POP Y
	Pop(areg->YA.B.Y);
	iapu->PC++;
}

void Apu0E(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// TSET1 abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	S9xAPUSetByte(Work8 | areg->YA.B.A, iapu->Address, iapu, apu);
	Work8 &= areg->YA.B.A;
	APUSetZN8(Work8);
	iapu->PC += 3;
}

void Apu4E(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// TCLR1 abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	S9xAPUSetByte(Work8 & ~areg->YA.B.A, iapu->Address, iapu, apu);
	Work8 &= areg->YA.B.A;
	APUSetZN8(Work8);
	iapu->PC += 3;
}

void Apu0F(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// BRK

#if 0
    STOP ("BRK");
#else
	PushW(iapu->PC + 1 - iapu->RAM);
	S9xAPUPackStatus_OP();
	Push(areg->P);
	APUSetBreak();
	APUClearInterrupt();
	// XXX:Where is the BRK vector ???
	iapu->PC = iapu->RAM + apu->ExtraRAM[0x20] + (apu->ExtraRAM[0x21] << 8);
#endif
}

void ApuEF(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SLEEP
	// XXX: sleep
	// STOP ("SLEEP");
	iapu->APUExecuting = FALSE;
	iapu->PC++;
}

void ApuFF(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// STOP
	// STOP ("STOP");
	iapu->APUExecuting = FALSE;
	iapu->PC++;
}

void Apu10(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// BPL
	Relative();
	if (!APUCheckNegative()) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
		APUShutdown();
	} else
		iapu->PC += 2;
}

void Apu30(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// BMI
	Relative();
	if (APUCheckNegative()) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
		APUShutdown();
	} else
		iapu->PC += 2;
}

void Apu90(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// BCC
	Relative();
	if (!APUCheckCarry()) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
		APUShutdown();
	} else
		iapu->PC += 2;
}

void ApuB0(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// BCS
	Relative();
	if (APUCheckCarry()) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
		APUShutdown();
	} else
		iapu->PC += 2;
}

void ApuD0(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// BNE
	Relative();
	if (!APUCheckZero()) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
		APUShutdown();
	} else
		iapu->PC += 2;
}

void ApuF0(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// BEQ
	Relative();
	if (APUCheckZero()) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
		APUShutdown();
	} else
		iapu->PC += 2;
}

void Apu50(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// BVC
	Relative();
	if (!APUCheckOverflow()) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
	} else
		iapu->PC += 2;
}

void Apu70(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// BVS
	Relative();
	if (APUCheckOverflow()) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
	} else
		iapu->PC += 2;
}

void Apu2F(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// BRA
	Relative();
	iapu->PC = iapu->RAM + (uint16)Int16;
}

void Apu80(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SETC
	APUSetCarry();
	iapu->PC++;
}

void ApuED(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// NOTC
	iapu->_Carry ^= 1;
	iapu->PC++;
}

void Apu40(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SETP
	APUSetDirectPage();
	iapu->DirectPage = iapu->RAM + 0x100;
	iapu->PC++;
}

void Apu1A(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DECW dp
	uint16 Work16 =
	    S9xAPUGetByteZ(OP1, iapu) + (S9xAPUGetByteZ(OP1 + 1, iapu) << 8);
	Work16--;
	S9xAPUSetByteZ((uint8)Work16, OP1, iapu, apu);
	S9xAPUSetByteZ(Work16 >> 8, OP1 + 1, iapu, apu);
	APUSetZN16(Work16);
	iapu->PC += 2;
}

void Apu5A(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMPW YA,dp
	uint16 Work16 =
	    S9xAPUGetByteZ(OP1, iapu) + (S9xAPUGetByteZ(OP1 + 1, iapu) << 8);
	int32 Int32 = (long)areg->YA.W - (long)Work16;
	iapu->_Carry = Int32 >= 0;
	APUSetZN16((uint16)Int32);
	iapu->PC += 2;
}

void Apu3A(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// INCW dp
	uint16 Work16 =
	    S9xAPUGetByteZ(OP1, iapu) + (S9xAPUGetByteZ(OP1 + 1, iapu) << 8);
	Work16++;
	S9xAPUSetByteZ((uint8)Work16, OP1, iapu, apu);
	S9xAPUSetByteZ(Work16 >> 8, OP1 + 1, iapu, apu);
	APUSetZN16(Work16);
	iapu->PC += 2;
}

void Apu7A(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADDW YA,dp
	uint16 Work16 =
	    S9xAPUGetByteZ(OP1, iapu) + (S9xAPUGetByteZ(OP1 + 1, iapu) << 8);
	uint32 Work32 = (uint32)areg->YA.W + Work16;
	APUClearHalfCarry();
	iapu->_Carry = Work32 >= 0x10000;
	if (~(areg->YA.W ^ Work16) & (Work16 ^ (uint16)Work32) & 0x8000)
		APUSetOverflow();
	else
		APUClearOverflow();
	areg->YA.W = (uint16)Work32;
	APUSetZN16(areg->YA.W);
	iapu->PC += 2;
}

void Apu9A(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SUBW YA,dp
	uint16 Work16 =
	    S9xAPUGetByteZ(OP1, iapu) + (S9xAPUGetByteZ(OP1 + 1, iapu) << 8);
	int32 Int32 = (long)areg->YA.W - (long)Work16;
	APUClearHalfCarry();
	iapu->_Carry = Int32 >= 0;
	if (((areg->YA.W ^ Work16) & 0x8000) &&
	    ((areg->YA.W ^ (uint16)Int32) & 0x8000))
		APUSetOverflow();
	else
		APUClearOverflow();
	if (((areg->YA.W ^ Work16) & 0x0080) &&
	    ((areg->YA.W ^ (uint16)Int32) & 0x0080))
		APUSetHalfCarry();
	areg->YA.W = (uint16)Int32;
	APUSetZN16(areg->YA.W);
	iapu->PC += 2;
}

void ApuBA(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOVW YA,dp
	areg->YA.B.A = S9xAPUGetByteZ(OP1, iapu);
	areg->YA.B.Y = S9xAPUGetByteZ(OP1 + 1, iapu);
	APUSetZN16(areg->YA.W);
	iapu->PC += 2;
}

void ApuDA(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOVW dp,YA
	S9xAPUSetByteZ(areg->YA.B.A, OP1, iapu, apu);
	S9xAPUSetByteZ(areg->YA.B.Y, OP1 + 1, iapu, apu);
	iapu->PC += 2;
}

void Apu64(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP A,dp
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	CMP(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void Apu65(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP A,abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	CMP(areg->YA.B.A, Work8);
	iapu->PC += 3;
}

void Apu66(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP A,(X)
	uint8 Work8 = S9xAPUGetByteZ(areg->X, iapu);
	CMP(areg->YA.B.A, Work8);
	iapu->PC++;
}

void Apu67(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP A,(dp+X)
	IndexedXIndirect();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	CMP(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void Apu68(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP A,#00
	uint8 Work8 = OP1;
	CMP(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void Apu69(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP dp(dest), dp(src)
	uint8 W1 = S9xAPUGetByteZ(OP1, iapu);
	uint8 Work8 = S9xAPUGetByteZ(OP2, iapu);
	CMP(Work8, W1);
	iapu->PC += 3;
}

void Apu74(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP A, dp+X
	uint8 Work8 = S9xAPUGetByteZ(OP1 + areg->X, iapu);
	CMP(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void Apu75(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP A,abs+X
	AbsoluteX();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	CMP(areg->YA.B.A, Work8);
	iapu->PC += 3;
}

void Apu76(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP A, abs+Y
	AbsoluteY();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	CMP(areg->YA.B.A, Work8);
	iapu->PC += 3;
}

void Apu77(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP A,(dp)+Y
	IndirectIndexedY();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	CMP(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void Apu78(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP dp,#00
	uint8 Work8 = OP1;
	uint8 W1 = S9xAPUGetByteZ(OP2, iapu);
	CMP(W1, Work8);
	iapu->PC += 3;
}

void Apu79(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP (X),(Y)
	uint8 W1 = S9xAPUGetByteZ(areg->X, iapu);
	uint8 Work8 = S9xAPUGetByteZ(areg->YA.B.Y, iapu);
	CMP(W1, Work8);
	iapu->PC++;
}

void Apu1E(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP X,abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	CMP(areg->X, Work8);
	iapu->PC += 3;
}

void Apu3E(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP X,dp
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	CMP(areg->X, Work8);
	iapu->PC += 2;
}

void ApuC8(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP X,#00
	CMP(areg->X, OP1);
	iapu->PC += 2;
}

void Apu5E(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP Y,abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	CMP(areg->YA.B.Y, Work8);
	iapu->PC += 3;
}

void Apu7E(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP Y,dp
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	CMP(areg->YA.B.Y, Work8);
	iapu->PC += 2;
}

void ApuAD(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CMP Y,#00
	uint8 Work8 = OP1;
	CMP(areg->YA.B.Y, Work8);
	iapu->PC += 2;
}

void Apu1F(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// JMP (abs+X)
	Absolute();
	iapu->PC = iapu->RAM + S9xAPUGetByte(iapu->Address + areg->X, iapu) +
		   (S9xAPUGetByte(iapu->Address + areg->X + 1, iapu) << 8);
	// XXX: HERE:
	// apu->Flags |= TRACE_FLAG;
}

void Apu5F(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// JMP abs
	Absolute();
	iapu->PC = iapu->RAM + iapu->Address;
}

void Apu20(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CLRP
	APUClearDirectPage();
	iapu->DirectPage = iapu->RAM;
	iapu->PC++;
}

void Apu60(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CLRC
	APUClearCarry();
	iapu->PC++;
}

void ApuE0(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CLRV
	APUClearHalfCarry();
	APUClearOverflow();
	iapu->PC++;
}

void Apu24(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND A,dp
	areg->YA.B.A &= S9xAPUGetByteZ(OP1, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu25(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND A,abs
	Absolute();
	areg->YA.B.A &= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void Apu26(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND A,(X)
	areg->YA.B.A &= S9xAPUGetByteZ(areg->X, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC++;
}

void Apu27(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND A,(dp+X)
	IndexedXIndirect();
	areg->YA.B.A &= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu28(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND A,#00
	areg->YA.B.A &= OP1;
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu29(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND dp(dest),dp(src)
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	Work8 &= S9xAPUGetByteZ(OP2, iapu);
	S9xAPUSetByteZ(Work8, OP2, iapu, apu);
	APUSetZN8(Work8);
	iapu->PC += 3;
}

void Apu34(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND A,dp+X
	areg->YA.B.A &= S9xAPUGetByteZ(OP1 + areg->X, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu35(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND A,abs+X
	AbsoluteX();
	areg->YA.B.A &= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void Apu36(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND A,abs+Y
	AbsoluteY();
	areg->YA.B.A &= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void Apu37(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND A,(dp)+Y
	IndirectIndexedY();
	areg->YA.B.A &= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu38(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND dp,#00
	uint8 Work8 = OP1;
	Work8 &= S9xAPUGetByteZ(OP2, iapu);
	S9xAPUSetByteZ(Work8, OP2, iapu, apu);
	APUSetZN8(Work8);
	iapu->PC += 3;
}

void Apu39(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// AND (X),(Y)
	uint8 Work8 =
	    S9xAPUGetByteZ(areg->X, iapu) & S9xAPUGetByteZ(areg->YA.B.Y, iapu);
	APUSetZN8(Work8);
	S9xAPUSetByteZ(Work8, areg->X, iapu, apu);
	iapu->PC++;
}

void Apu2B(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ROL dp
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	ROL(Work8);
	S9xAPUSetByteZ(Work8, OP1, iapu, apu);
	iapu->PC += 2;
}

void Apu2C(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ROL abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	ROL(Work8);
	S9xAPUSetByte(Work8, iapu->Address, iapu, apu);
	iapu->PC += 3;
}

void Apu3B(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ROL dp+X
	uint8 Work8 = S9xAPUGetByteZ(OP1 + areg->X, iapu);
	ROL(Work8);
	S9xAPUSetByteZ(Work8, OP1 + areg->X, iapu, apu);
	iapu->PC += 2;
}

void Apu3C(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ROL A
	ROL(areg->YA.B.A);
	iapu->PC++;
}

void Apu2E(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CBNE dp,rel
	uint8 Work8 = OP1;
	Relative2();

	if (S9xAPUGetByteZ(Work8, iapu) != areg->YA.B.A) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
		APUShutdown();
	} else
		iapu->PC += 3;
}

void ApuDE(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// CBNE dp+X,rel
	uint8 Work8 = OP1 + areg->X;
	Relative2();

	if (S9xAPUGetByteZ(Work8, iapu) != areg->YA.B.A) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
		APUShutdown();
	} else
		iapu->PC += 3;
}

void Apu3D(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// INC X
	areg->X = ((areg->X + 1) & 0xff);
	APUSetZN8(areg->X);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC++;
}

void ApuFC(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// INC Y
	areg->YA.B.Y++;
	APUSetZN8(areg->YA.B.Y);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC++;
}

void Apu1D(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DEC X
	areg->X = ((areg->X - 1) & 0xff);
	APUSetZN8(areg->X);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC++;
}

void ApuDC(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DEC Y
	areg->YA.B.Y--;
	APUSetZN8(areg->YA.B.Y);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC++;
}

void ApuAB(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// INC dp
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu) + 1;
	S9xAPUSetByteZ(Work8, OP1, iapu, apu);
	APUSetZN8(Work8);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC += 2;
}

void ApuAC(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// INC abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu) + 1;
	S9xAPUSetByte(Work8, iapu->Address, iapu, apu);
	APUSetZN8(Work8);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC += 3;
}

void ApuBB(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// INC dp+X
	uint8 Work8 = S9xAPUGetByteZ(OP1 + areg->X, iapu) + 1;
	S9xAPUSetByteZ(Work8, OP1 + areg->X, iapu, apu);
	APUSetZN8(Work8);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC += 2;
}

void ApuBC(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// INC A
	areg->YA.B.A++;
	APUSetZN8(areg->YA.B.A);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC++;
}

void Apu8B(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DEC dp
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu) - 1;
	S9xAPUSetByteZ(Work8, OP1, iapu, apu);
	APUSetZN8(Work8);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC += 2;
}

void Apu8C(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DEC abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu) - 1;
	S9xAPUSetByte(Work8, iapu->Address, iapu, apu);
	APUSetZN8(Work8);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC += 3;
}

void Apu9B(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DEC dp+X
	uint8 Work8 = S9xAPUGetByteZ(OP1 + areg->X, iapu) - 1;
	S9xAPUSetByteZ(Work8, OP1 + areg->X, iapu, apu);
	APUSetZN8(Work8);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC += 2;
}

void Apu9C(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DEC A
	areg->YA.B.A--;
	APUSetZN8(areg->YA.B.A);

#ifdef SPC700_SHUTDOWN
	iapu->WaitCounter++;
#endif

	iapu->PC++;
}

void Apu44(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR A,dp
	areg->YA.B.A ^= S9xAPUGetByteZ(OP1, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu45(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR A,abs
	Absolute();
	areg->YA.B.A ^= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void Apu46(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR A,(X)
	areg->YA.B.A ^= S9xAPUGetByteZ(areg->X, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC++;
}

void Apu47(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR A,(dp+X)
	IndexedXIndirect();
	areg->YA.B.A ^= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu48(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR A,#00
	areg->YA.B.A ^= OP1;
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu49(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR dp(dest),dp(src)
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	Work8 ^= S9xAPUGetByteZ(OP2, iapu);
	S9xAPUSetByteZ(Work8, OP2, iapu, apu);
	APUSetZN8(Work8);
	iapu->PC += 3;
}

void Apu54(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR A,dp+X
	areg->YA.B.A ^= S9xAPUGetByteZ(OP1 + areg->X, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu55(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR A,abs+X
	AbsoluteX();
	areg->YA.B.A ^= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void Apu56(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR A,abs+Y
	AbsoluteY();
	areg->YA.B.A ^= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void Apu57(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR A,(dp)+Y
	IndirectIndexedY();
	areg->YA.B.A ^= S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void Apu58(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR dp,#00
	uint8 Work8 = OP1;
	Work8 ^= S9xAPUGetByteZ(OP2, iapu);
	S9xAPUSetByteZ(Work8, OP2, iapu, apu);
	APUSetZN8(Work8);
	iapu->PC += 3;
}

void Apu59(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EOR (X),(Y)
	uint8 Work8 =
	    S9xAPUGetByteZ(areg->X, iapu) ^ S9xAPUGetByteZ(areg->YA.B.Y, iapu);
	APUSetZN8(Work8);
	S9xAPUSetByteZ(Work8, areg->X, iapu, apu);
	iapu->PC++;
}

void Apu4B(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// LSR dp
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	LSR(Work8);
	S9xAPUSetByteZ(Work8, OP1, iapu, apu);
	iapu->PC += 2;
}

void Apu4C(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// LSR abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	LSR(Work8);
	S9xAPUSetByte(Work8, iapu->Address, iapu, apu);
	iapu->PC += 3;
}

void Apu5B(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// LSR dp+X
	uint8 Work8 = S9xAPUGetByteZ(OP1 + areg->X, iapu);
	LSR(Work8);
	S9xAPUSetByteZ(Work8, OP1 + areg->X, iapu, apu);
	iapu->PC += 2;
}

void Apu5C(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// LSR A
	LSR(areg->YA.B.A);
	iapu->PC++;
}

void Apu7D(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A,X
	areg->YA.B.A = areg->X;
	APUSetZN8(areg->YA.B.A);
	iapu->PC++;
}

void ApuDD(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A,Y
	areg->YA.B.A = areg->YA.B.Y;
	APUSetZN8(areg->YA.B.A);
	iapu->PC++;
}

void Apu5D(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV X,A
	areg->X = areg->YA.B.A;
	APUSetZN8(areg->X);
	iapu->PC++;
}

void ApuFD(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV Y,A
	areg->YA.B.Y = areg->YA.B.A;
	APUSetZN8(areg->YA.B.Y);
	iapu->PC++;
}

void Apu9D(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV X,SP
	areg->X = ((areg->S) & 0xff);
	APUSetZN8(areg->X);
	iapu->PC++;
}

void ApuBD(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV SP,X
	areg->S = ((areg->X) & 0xff);
	iapu->PC++;
}

void Apu6B(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ROR dp
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	ROR(Work8);
	S9xAPUSetByteZ(Work8, OP1, iapu, apu);
	iapu->PC += 2;
}

void Apu6C(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ROR abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	ROR(Work8);
	S9xAPUSetByte(Work8, iapu->Address, iapu, apu);
	iapu->PC += 3;
}

void Apu7B(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ROR dp+X
	uint8 Work8 = S9xAPUGetByteZ(OP1 + areg->X, iapu);
	ROR(Work8);
	S9xAPUSetByteZ(Work8, OP1 + areg->X, iapu, apu);
	iapu->PC += 2;
}

void Apu7C(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ROR A
	ROR(areg->YA.B.A);
	iapu->PC++;
}

void Apu6E(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DBNZ dp,rel
	uint8 Work8 = OP1;
	Relative2();
	uint8 W1 = S9xAPUGetByteZ(Work8, iapu) - 1;
	S9xAPUSetByteZ(W1, Work8, iapu, apu);
	if (W1 != 0) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
	} else
		iapu->PC += 3;
}

void ApuFE(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DBNZ Y,rel
	Relative();
	areg->YA.B.Y--;
	if (areg->YA.B.Y != 0) {
		iapu->PC = iapu->RAM + (uint16)Int16;
		apu->Cycles += iapu->TwoCycles;
	} else
		iapu->PC += 2;
}

void Apu6F(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// RET
	PopW(areg->PC);
	iapu->PC = iapu->RAM + areg->PC;
}

void Apu7F(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// RETI
	// STOP ("RETI");
	Pop(areg->P);
	S9xAPUUnpackStatus_OP();
	PopW(areg->PC);
	iapu->PC = iapu->RAM + areg->PC;
}

void Apu84(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC A,dp
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	ADC(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void Apu85(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC A, abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	ADC(areg->YA.B.A, Work8);
	iapu->PC += 3;
}

void Apu86(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC A,(X)
	uint8 Work8 = S9xAPUGetByteZ(areg->X, iapu);
	ADC(areg->YA.B.A, Work8);
	iapu->PC++;
}

void Apu87(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC A,(dp+X)
	IndexedXIndirect();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	ADC(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void Apu88(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC A,#00
	uint8 Work8 = OP1;
	ADC(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void Apu89(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC dp(dest),dp(src)
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	uint8 W1 = S9xAPUGetByteZ(OP2, iapu);
	ADC(W1, Work8);
	S9xAPUSetByteZ(W1, OP2, iapu, apu);
	iapu->PC += 3;
}

void Apu94(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC A,dp+X
	uint8 Work8 = S9xAPUGetByteZ(OP1 + areg->X, iapu);
	ADC(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void Apu95(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC A, abs+X
	AbsoluteX();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	ADC(areg->YA.B.A, Work8);
	iapu->PC += 3;
}

void Apu96(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC A, abs+Y
	AbsoluteY();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	ADC(areg->YA.B.A, Work8);
	iapu->PC += 3;
}

void Apu97(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC A, (dp)+Y
	IndirectIndexedY();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	ADC(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void Apu98(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC dp,#00
	uint8 Work8 = OP1;
	uint8 W1 = S9xAPUGetByteZ(OP2, iapu);
	ADC(W1, Work8);
	S9xAPUSetByteZ(W1, OP2, iapu, apu);
	iapu->PC += 3;
}

void Apu99(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// ADC (X),(Y)
	uint8 W1 = S9xAPUGetByteZ(areg->X, iapu);
	uint8 Work8 = S9xAPUGetByteZ(areg->YA.B.Y, iapu);
	ADC(W1, Work8);
	S9xAPUSetByteZ(W1, areg->X, iapu, apu);
	iapu->PC++;
}

void Apu8D(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV Y,#00
	areg->YA.B.Y = OP1;
	APUSetZN8(areg->YA.B.Y);
	iapu->PC += 2;
}

void Apu8F(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV dp,#00
	uint8 Work8 = OP1;
	S9xAPUSetByteZ(Work8, OP2, iapu, apu);
	iapu->PC += 3;
}

void Apu9E(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DIV YA,X
	if (areg->X == 0) {
		APUSetOverflow();
		areg->YA.B.Y = 0xff;
		areg->YA.B.A = 0xff;
	} else {
		APUClearOverflow();
		uint8 Work8 = areg->YA.W / areg->X;
		areg->YA.B.Y = areg->YA.W % areg->X;
		areg->YA.B.A = Work8;
	}
	// XXX How should Overflow, Half Carry, Zero and Negative flags be set??
	// APUSetZN16 (areg->YA.W);
	APUSetZN8(areg->YA.B.A);
	iapu->PC++;
}

void Apu9F(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// XCN A
	areg->YA.B.A = (areg->YA.B.A >> 4) | (areg->YA.B.A << 4);
	APUSetZN8(areg->YA.B.A);
	iapu->PC++;
}

void ApuA4(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC A, dp
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	SBC(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void ApuA5(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC A, abs
	Absolute();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	SBC(areg->YA.B.A, Work8);
	iapu->PC += 3;
}

void ApuA6(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC A, (X)
	uint8 Work8 = S9xAPUGetByteZ(areg->X, iapu);
	SBC(areg->YA.B.A, Work8);
	iapu->PC++;
}

void ApuA7(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC A,(dp+X)
	IndexedXIndirect();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	SBC(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void ApuA8(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC A,#00
	uint8 Work8 = OP1;
	SBC(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void ApuA9(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC dp(dest), dp(src)
	uint8 Work8 = S9xAPUGetByteZ(OP1, iapu);
	uint8 W1 = S9xAPUGetByteZ(OP2, iapu);
	SBC(W1, Work8);
	S9xAPUSetByteZ(W1, OP2, iapu, apu);
	iapu->PC += 3;
}

void ApuB4(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC A, dp+X
	uint8 Work8 = S9xAPUGetByteZ(OP1 + areg->X, iapu);
	SBC(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void ApuB5(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC A,abs+X
	AbsoluteX();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	SBC(areg->YA.B.A, Work8);
	iapu->PC += 3;
}

void ApuB6(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC A,abs+Y
	AbsoluteY();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	SBC(areg->YA.B.A, Work8);
	iapu->PC += 3;
}

void ApuB7(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC A,(dp)+Y
	IndirectIndexedY();
	uint8 Work8 = S9xAPUGetByte(iapu->Address, iapu);
	SBC(areg->YA.B.A, Work8);
	iapu->PC += 2;
}

void ApuB8(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC dp,#00
	uint8 Work8 = OP1;
	uint8 W1 = S9xAPUGetByteZ(OP2, iapu);
	SBC(W1, Work8);
	S9xAPUSetByteZ(W1, OP2, iapu, apu);
	iapu->PC += 3;
}

void ApuB9(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// SBC (X),(Y)
	uint8 W1 = S9xAPUGetByteZ(areg->X, iapu);
	uint8 Work8 = S9xAPUGetByteZ(areg->YA.B.Y, iapu);
	SBC(W1, Work8);
	S9xAPUSetByteZ(W1, areg->X, iapu, apu);
	iapu->PC++;
}

void ApuAF(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV (X)+, A
	S9xAPUSetByteZ(areg->YA.B.A, areg->X++, iapu, apu);
	iapu->PC++;
}

void ApuBE(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DAS
	// Implemented by Jonathan Gevaryahu (using the DAA instruction code)
	uint8 W1 = areg->YA.B.A & 0xf;
	uint8 W2 = areg->YA.B.A >> 4;
	APUClearCarry();
	if (W1 > 9) {
		W1 -= 6;
	}
	if (W2 > 9) // This should never happen....
	{
		W2 -= 6;
		APUSetCarry();
	}
	areg->YA.B.A = W1 | (W2 << 4);
	iapu->PC++;
}

void ApuBF(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A,(X)+
	areg->YA.B.A = S9xAPUGetByteZ(areg->X++, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC++;
}

void ApuC0(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DI
	APUClearInterrupt();
	iapu->PC++;
}

void ApuA0(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// EI
	APUSetInterrupt();
	iapu->PC++;
}

void ApuC4(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV dp,A
	S9xAPUSetByteZ(areg->YA.B.A, OP1, iapu, apu);
	iapu->PC += 2;
}

void ApuC5(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV abs,A
	Absolute();
	S9xAPUSetByte(areg->YA.B.A, iapu->Address, iapu, apu);
	iapu->PC += 3;
}

void ApuC6(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV (X), A
	S9xAPUSetByteZ(areg->YA.B.A, areg->X, iapu, apu);
	iapu->PC++;
}

void ApuC7(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV (dp+X),A
	IndexedXIndirect();
	S9xAPUSetByte(areg->YA.B.A, iapu->Address, iapu, apu);
	iapu->PC += 2;
}

void ApuC9(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV abs,X
	Absolute();
	S9xAPUSetByte(areg->X, iapu->Address, iapu, apu);
	iapu->PC += 3;
}

void ApuCB(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV dp,Y
	S9xAPUSetByteZ(areg->YA.B.Y, OP1, iapu, apu);
	iapu->PC += 2;
}

void ApuCC(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV abs,Y
	Absolute();
	S9xAPUSetByte(areg->YA.B.Y, iapu->Address, iapu, apu);
	iapu->PC += 3;
}

void ApuCD(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV X,#00
	areg->X = OP1;
	APUSetZN8(areg->X);
	iapu->PC += 2;
}

void ApuCF(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MUL YA
	areg->YA.W = (uint16)areg->YA.B.A * areg->YA.B.Y;
	APUSetZN16(areg->YA.W);
	iapu->PC++;
}

void ApuD4(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV dp+X, A
	S9xAPUSetByteZ(areg->YA.B.A, OP1 + areg->X, iapu, apu);
	iapu->PC += 2;
}

void ApuD5(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV abs+X,A
	AbsoluteX();
	S9xAPUSetByte(areg->YA.B.A, iapu->Address, iapu, apu);
	iapu->PC += 3;
}

void ApuD6(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV abs+Y,A
	AbsoluteY();
	S9xAPUSetByte(areg->YA.B.A, iapu->Address, iapu, apu);
	iapu->PC += 3;
}

void ApuD7(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV (dp)+Y,A
	IndirectIndexedY();
	S9xAPUSetByte(areg->YA.B.A, iapu->Address, iapu, apu);
	iapu->PC += 2;
}

void ApuD8(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV dp,X
	S9xAPUSetByteZ(areg->X, OP1, iapu, apu);
	iapu->PC += 2;
}

void ApuD9(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV dp+Y,X
	S9xAPUSetByteZ(areg->X, OP1 + areg->YA.B.Y, iapu, apu);
	iapu->PC += 2;
}

void ApuDB(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV dp+X,Y
	S9xAPUSetByteZ(areg->YA.B.Y, OP1 + areg->X, iapu, apu);
	iapu->PC += 2;
}

void ApuDF(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// DAA
	uint8 W1 = areg->YA.B.A & 0xf;
	uint8 W2 = areg->YA.B.A >> 4;
	APUClearCarry();
	if (W1 > 9) {
		W1 -= 6;
	}
	if (W2 > 9) {
		W2 -= 6;
		APUSetCarry();
	}
	areg->YA.B.A = W1 | (W2 << 4);
	APUSetZN8(areg->YA.B.A);
	iapu->PC++;
}

void ApuE4(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A, dp
	areg->YA.B.A = S9xAPUGetByteZ(OP1, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void ApuE5(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A,abs
	Absolute();
	areg->YA.B.A = S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void ApuE6(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A,(X)
	areg->YA.B.A = S9xAPUGetByteZ(areg->X, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC++;
}

void ApuE7(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A,(dp+X)
	IndexedXIndirect();
	areg->YA.B.A = S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void ApuE8(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A,#00
	areg->YA.B.A = OP1;
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void ApuE9(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV X, abs
	Absolute();
	areg->X = S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->X);
	iapu->PC += 3;
}

void ApuEB(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV Y,dp
	areg->YA.B.Y = S9xAPUGetByteZ(OP1, iapu);
	APUSetZN8(areg->YA.B.Y);
	iapu->PC += 2;
}

void ApuEC(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV Y,abs
	Absolute();
	areg->YA.B.Y = S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.Y);
	iapu->PC += 3;
}

void ApuF4(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A, dp+X
	areg->YA.B.A = S9xAPUGetByteZ(OP1 + areg->X, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void ApuF5(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A, abs+X
	AbsoluteX();
	areg->YA.B.A = S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void ApuF6(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A, abs+Y
	AbsoluteY();
	areg->YA.B.A = S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 3;
}

void ApuF7(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV A, (dp)+Y
	IndirectIndexedY();
	areg->YA.B.A = S9xAPUGetByte(iapu->Address, iapu);
	APUSetZN8(areg->YA.B.A);
	iapu->PC += 2;
}

void ApuF8(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV X,dp
	areg->X = S9xAPUGetByteZ(OP1, iapu);
	APUSetZN8(areg->X);
	iapu->PC += 2;
}

void ApuF9(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV X,dp+Y
	areg->X = S9xAPUGetByteZ(OP1 + areg->YA.B.Y, iapu);
	APUSetZN8(areg->X);
	iapu->PC += 2;
}

void ApuFA(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV dp(dest),dp(src)
	S9xAPUSetByteZ(S9xAPUGetByteZ(OP1, iapu), OP2, iapu, apu);
	iapu->PC += 3;
}

void ApuFB(struct SAPURegisters *areg, struct SIAPU *iapu, struct SAPU *apu)
{
	// MOV Y,dp+X
	areg->YA.B.Y = S9xAPUGetByteZ(OP1 + areg->X, iapu);
	APUSetZN8(areg->YA.B.Y);
	iapu->PC += 2;
}

#ifdef NO_INLINE_SET_GET
#undef INLINE
#define INLINE
#include "apumem.h"
#endif

void (*S9xApuOpcodes[256])(struct SAPURegisters *, struct SIAPU *,
			   struct SAPU *) = {
    Apu00, Apu01, Apu02, Apu03, Apu04, Apu05, Apu06, Apu07, Apu08, Apu09, Apu0A,
    Apu0B, Apu0C, Apu0D, Apu0E, Apu0F, Apu10, Apu11, Apu12, Apu13, Apu14, Apu15,
    Apu16, Apu17, Apu18, Apu19, Apu1A, Apu1B, Apu1C, Apu1D, Apu1E, Apu1F, Apu20,
    Apu21, Apu22, Apu23, Apu24, Apu25, Apu26, Apu27, Apu28, Apu29, Apu2A, Apu2B,
    Apu2C, Apu2D, Apu2E, Apu2F, Apu30, Apu31, Apu32, Apu33, Apu34, Apu35, Apu36,
    Apu37, Apu38, Apu39, Apu3A, Apu3B, Apu3C, Apu3D, Apu3E, Apu3F, Apu40, Apu41,
    Apu42, Apu43, Apu44, Apu45, Apu46, Apu47, Apu48, Apu49, Apu4A, Apu4B, Apu4C,
    Apu4D, Apu4E, Apu4F, Apu50, Apu51, Apu52, Apu53, Apu54, Apu55, Apu56, Apu57,
    Apu58, Apu59, Apu5A, Apu5B, Apu5C, Apu5D, Apu5E, Apu5F, Apu60, Apu61, Apu62,
    Apu63, Apu64, Apu65, Apu66, Apu67, Apu68, Apu69, Apu6A, Apu6B, Apu6C, Apu6D,
    Apu6E, Apu6F, Apu70, Apu71, Apu72, Apu73, Apu74, Apu75, Apu76, Apu77, Apu78,
    Apu79, Apu7A, Apu7B, Apu7C, Apu7D, Apu7E, Apu7F, Apu80, Apu81, Apu82, Apu83,
    Apu84, Apu85, Apu86, Apu87, Apu88, Apu89, Apu8A, Apu8B, Apu8C, Apu8D, Apu8E,
    Apu8F, Apu90, Apu91, Apu92, Apu93, Apu94, Apu95, Apu96, Apu97, Apu98, Apu99,
    Apu9A, Apu9B, Apu9C, Apu9D, Apu9E, Apu9F, ApuA0, ApuA1, ApuA2, ApuA3, ApuA4,
    ApuA5, ApuA6, ApuA7, ApuA8, ApuA9, ApuAA, ApuAB, ApuAC, ApuAD, ApuAE, ApuAF,
    ApuB0, ApuB1, ApuB2, ApuB3, ApuB4, ApuB5, ApuB6, ApuB7, ApuB8, ApuB9, ApuBA,
    ApuBB, ApuBC, ApuBD, ApuBE, ApuBF, ApuC0, ApuC1, ApuC2, ApuC3, ApuC4, ApuC5,
    ApuC6, ApuC7, ApuC8, ApuC9, ApuCA, ApuCB, ApuCC, ApuCD, ApuCE, ApuCF, ApuD0,
    ApuD1, ApuD2, ApuD3, ApuD4, ApuD5, ApuD6, ApuD7, ApuD8, ApuD9, ApuDA, ApuDB,
    ApuDC, ApuDD, ApuDE, ApuDF, ApuE0, ApuE1, ApuE2, ApuE3, ApuE4, ApuE5, ApuE6,
    ApuE7, ApuE8, ApuE9, ApuEA, ApuEB, ApuEC, ApuED, ApuEE, ApuEF, ApuF0, ApuF1,
    ApuF2, ApuF3, ApuF4, ApuF5, ApuF6, ApuF7, ApuF8, ApuF9, ApuFA, ApuFB, ApuFC,
    ApuFD, ApuFE, ApuFF};
