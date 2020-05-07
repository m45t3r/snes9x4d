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
#ifndef _CPUMACRO_H_
#define _CPUMACRO_H_

#define SETZN16(W) \
    icpu->_Zero = (W); \
    icpu->_Negative = (uint8) ((W) >> 8);

#define SETZN8(W) \
    icpu->_Zero = (W); \
    icpu->_Negative = (W); \

/*
STATIC inline void SETZN16 (uint16 Work)
{
    ICPU._Zero = Work != 0;
    ICPU._Negative = (uint8) (Work >> 8);
}

STATIC inline void SETZN8 (uint8 Work)
{
    ICPU._Zero = Work;
    ICPU._Negative = Work;
}
*/

STATIC inline void ADC8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint8 Work8 = S9xGetByte (OpAddress, cpu);
    
    if (CHECKDECIMAL())
    {
	uint8 A1 = (reg->A.W) & 0xF;
	uint8 A2 = (reg->A.W >> 4) & 0xF;
	uint8 W1 = Work8 & 0xF;
	uint8 W2 = (Work8 >> 4) & 0xF;

	A1 += W1 + CHECKCARRY();
	if (A1 > 9)
	{
	    A1 -= 10;
	    A2++;
	}

	A2 += W2;
	if (A2 > 9)
	{
	    A2 -= 10;
	    SETCARRY ();
	}
	else
	{
	    CLEARCARRY ();
	}

	uint8 Ans8 = (A2 << 4) | A1;
	if (~(reg->AL ^ Work8) &
	    (Work8 ^ Ans8) & 0x80)
	     SETOVERFLOW ();
	else
	    CLEAROVERFLOW();
	reg->AL = Ans8;
	SETZN8 (reg->AL);
    }
    else
    {
	uint16 Ans16 = reg->AL + Work8 + CHECKCARRY();

	icpu->_Carry = Ans16 >= 0x100;

	if (~(reg->AL ^ Work8) & 
	     (Work8 ^ (uint8) Ans16) & 0x80)
	    SETOVERFLOW();
	else
	    CLEAROVERFLOW();
	reg->AL = (uint8) Ans16;
	SETZN8 (reg->AL);

    }
}

STATIC inline void ADC16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint16 Work16 = S9xGetWord (OpAddress, cpu);

    if (CHECKDECIMAL())
    {
	uint8 A1 = (reg->A.W) & 0xF;
	uint8 A2 = (reg->A.W >> 4) & 0xF;
	uint8 A3 = (reg->A.W >> 8) & 0xF;
	uint8 A4 = (reg->A.W >> 12) & 0xF;
	uint8 W1 = Work16 & 0xF;
	uint8 W2 = (Work16 >> 4) & 0xF;
	uint8 W3 = (Work16 >> 8) & 0xF;
	uint8 W4 = (Work16 >> 12) & 0xF;

	A1 += W1 + CHECKCARRY ();
	if (A1 > 9)
	{
	    A1 -= 10;
	    A2++;
	}

	A2 += W2;
	if (A2 > 9)
	{
	    A2 -= 10;
	    A3++;
	}

	A3 += W3;
	if (A3 > 9)
	{
	    A3 -= 10;
	    A4++;
	}

	A4 += W4;
	if (A4 > 9)
	{
	    A4 -= 10;
	    SETCARRY ();
	}
	else
	{
	    CLEARCARRY ();
	}

	uint16 Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
	if (~(reg->A.W ^ Work16) &
	    (Work16 ^ Ans16) & 0x8000)
	    SETOVERFLOW();
	else
	    CLEAROVERFLOW();
	reg->A.W = Ans16;
	SETZN16 (reg->A.W);
    }
    else
    {
	uint32 Ans32 = reg->A.W + Work16 + CHECKCARRY();

	icpu->_Carry = Ans32 >= 0x10000;

	if (~(reg->A.W ^ Work16) &
	    (Work16 ^ (uint16) Ans32) & 0x8000)
	    SETOVERFLOW();
	else
	    CLEAROVERFLOW();
	reg->A.W = (uint16) Ans32;
	SETZN16 (reg->A.W);
    }
}

STATIC inline void AND16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->A.W &= S9xGetWord (OpAddress, cpu);
    SETZN16 (reg->A.W);
}

STATIC inline void AND8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->AL &= S9xGetByte (OpAddress, cpu);
    SETZN8 (reg->AL);
}

STATIC inline void A_ASL16 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    icpu->_Carry = (reg->AH & 0x80) != 0;
    reg->A.W <<= 1;
    SETZN16 (reg->A.W);
}

STATIC inline void A_ASL8 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    icpu->_Carry = (reg->AL & 0x80) != 0;
    reg->AL <<= 1;
    SETZN8 (reg->AL);
}

STATIC inline void ASL16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress, cpu);
    icpu->_Carry = (Work16 & 0x8000) != 0;
    Work16 <<= 1;
    S9xSetWord (Work16, OpAddress, cpu);
    SETZN16 (Work16);
}

STATIC inline void ASL8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress, cpu);
    icpu->_Carry = (Work8 & 0x80) != 0;
    Work8 <<= 1;
    S9xSetByte (Work8, OpAddress, cpu);
    SETZN8 (Work8);
}

STATIC inline void BIT16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint16 Work16 = S9xGetWord (OpAddress, cpu);
    icpu->_Overflow = (Work16 & 0x4000) != 0;
    icpu->_Negative = (uint8) (Work16 >> 8);
    icpu->_Zero = (Work16 & reg->A.W) != 0;
}

STATIC inline void BIT8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint8 Work8 = S9xGetByte (OpAddress, cpu);
    icpu->_Overflow = (Work8 & 0x40) != 0;
    icpu->_Negative = Work8;
    icpu->_Zero = Work8 & reg->AL;
}

STATIC inline void CMP16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    int32 Int32 = (long) reg->A.W -
	    (long) S9xGetWord (OpAddress, cpu);
    icpu->_Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
}

STATIC inline void CMP8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    int32 Int32 = (short) reg->AL -
	    (short) S9xGetByte (OpAddress, cpu);
    icpu->_Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
}

STATIC inline void CMX16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    int32 Int32 = (long) reg->X.W -
	    (long) S9xGetWord (OpAddress, cpu);
    icpu->_Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
}

STATIC inline void CMX8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    int32 Int32 = (short) reg->XL -
	    (short) S9xGetByte (OpAddress, cpu);
    icpu->_Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
}

STATIC inline void CMY16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    int32 Int32 = (long) reg->Y.W -
	    (long) S9xGetWord (OpAddress, cpu);
    icpu->_Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
}

STATIC inline void CMY8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    int32 Int32 = (short) reg->YL -
	    (short) S9xGetByte (OpAddress, cpu);
    icpu->_Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
}

STATIC inline void A_DEC16 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->A.W--;
    SETZN16 (reg->A.W);
}

STATIC inline void A_DEC8 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->AL--;
    SETZN8 (reg->AL);
}

STATIC inline void DEC16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    uint16 Work16 = S9xGetWord (OpAddress, cpu) - 1;
    S9xSetWord (Work16, OpAddress, cpu);
    SETZN16 (Work16);
}

STATIC inline void DEC8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    uint8 Work8 = S9xGetByte (OpAddress, cpu) - 1;
    S9xSetByte (Work8, OpAddress, cpu);
    SETZN8 (Work8);
}

STATIC inline void EOR16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->A.W ^= S9xGetWord (OpAddress, cpu);
    SETZN16 (reg->A.W);
}

STATIC inline void EOR8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->AL ^= S9xGetByte (OpAddress, cpu);
    SETZN8 (reg->AL);
}

STATIC inline void A_INC16 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->A.W++;
    SETZN16 (reg->A.W);
}

STATIC inline void A_INC8 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    reg->AL++;
    SETZN8 (reg->AL);
}

STATIC inline void INC16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    uint16 Work16 = S9xGetWord (OpAddress, cpu) + 1;
    S9xSetWord (Work16, OpAddress, cpu);
    SETZN16 (Work16);
}

STATIC inline void INC8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    cpu->WaitAddress = NULL;
#endif

    uint8 Work8 = S9xGetByte (OpAddress, cpu) + 1;
    S9xSetByte (Work8, OpAddress, cpu);
    SETZN8 (Work8);
}

STATIC inline void LDA16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->A.W = S9xGetWord (OpAddress, cpu);
    SETZN16 (reg->A.W);
}

STATIC inline void LDA8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->AL = S9xGetByte (OpAddress, cpu);
    SETZN8 (reg->AL);
}

STATIC inline void LDX16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->X.W = S9xGetWord (OpAddress, cpu);
    SETZN16 (reg->X.W);
}

STATIC inline void LDX8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->XL = S9xGetByte (OpAddress, cpu);
    SETZN8 (reg->XL);
}

STATIC inline void LDY16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->Y.W = S9xGetWord (OpAddress, cpu);
    SETZN16 (reg->Y.W);
}

STATIC inline void LDY8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->YL = S9xGetByte (OpAddress, cpu);
    SETZN8 (reg->YL);
}

STATIC inline void A_LSR16 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    icpu->_Carry = reg->AL & 1;
    reg->A.W >>= 1;
    SETZN16 (reg->A.W);
}

STATIC inline void A_LSR8 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    icpu->_Carry = reg->AL & 1;
    reg->AL >>= 1;
    SETZN8 (reg->AL);
}

STATIC inline void LSR16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress, cpu);
    icpu->_Carry = Work16 & 1;
    Work16 >>= 1;
    S9xSetWord (Work16, OpAddress, cpu);
    SETZN16 (Work16);
}

STATIC inline void LSR8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress, cpu);
    icpu->_Carry = Work8 & 1;
    Work8 >>= 1;
    S9xSetByte (Work8, OpAddress, cpu);
    SETZN8 (Work8);
}

STATIC inline void ORA16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->A.W |= S9xGetWord (OpAddress, cpu);
    SETZN16 (reg->A.W);
}

STATIC inline void ORA8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    reg->AL |= S9xGetByte (OpAddress, cpu);
    SETZN8 (reg->AL);
}

STATIC inline void A_ROL16 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = (reg->A.W << 1) | CHECKCARRY();
    icpu->_Carry = Work32 >= 0x10000;
    reg->A.W = (uint16) Work32;
    SETZN16 ((uint16) Work32);
}

STATIC inline void A_ROL8 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = reg->AL;
    Work16 <<= 1;
    Work16 |= CHECKCARRY();
    icpu->_Carry = Work16 >= 0x100;
    reg->AL = (uint8) Work16;
    SETZN8 ((uint8) Work16);
}

STATIC inline void ROL16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = S9xGetWord (OpAddress, cpu);
    Work32 <<= 1;
    Work32 |= CHECKCARRY();
    icpu->_Carry = Work32 >= 0x10000;
    S9xSetWord ((uint16) Work32, OpAddress, cpu);
    SETZN16 ((uint16) Work32);
}

STATIC inline void ROL8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetByte (OpAddress, cpu);
    Work16 <<= 1;
    Work16 |= CHECKCARRY ();
    icpu->_Carry = Work16 >= 0x100;
    S9xSetByte ((uint8) Work16, OpAddress, cpu);
    SETZN8 ((uint8) Work16);
}

STATIC inline void A_ROR16 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = reg->A.W;
    Work32 |= (int) CHECKCARRY() << 16;
    icpu->_Carry = (uint8) (Work32 & 1);
    Work32 >>= 1;
    reg->A.W = (uint16) Work32;
    SETZN16 ((uint16) Work32);
}

STATIC inline void A_ROR8 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = reg->AL | ((uint16) CHECKCARRY() << 8);
    icpu->_Carry = (uint8) Work16 & 1;
    Work16 >>= 1;
    reg->AL = (uint8) Work16;
    SETZN8 ((uint8) Work16);
}

STATIC inline void ROR16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = S9xGetWord (OpAddress, cpu);
    Work32 |= (int) CHECKCARRY() << 16;
    icpu->_Carry = (uint8) (Work32 & 1);
    Work32 >>= 1;
    S9xSetWord ((uint16) Work32, OpAddress, cpu);
    SETZN16 ((uint16) Work32);
}

STATIC inline void ROR8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetByte (OpAddress, cpu);
    Work16 |= (int) CHECKCARRY () << 8;
    icpu->_Carry = (uint8) (Work16 & 1);
    Work16 >>= 1;
    S9xSetByte ((uint8) Work16, OpAddress, cpu);
    SETZN8 ((uint8) Work16);
}

STATIC inline void SBC16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint16 Work16 = S9xGetWord (OpAddress, cpu);

    if (CHECKDECIMAL())
    {
	uint8 A1 = (reg->A.W) & 0xF;
	uint8 A2 = (reg->A.W >> 4) & 0xF;
	uint8 A3 = (reg->A.W >> 8) & 0xF;
	uint8 A4 = (reg->A.W >> 12) & 0xF;
	uint8 W1 = Work16 & 0xF;
	uint8 W2 = (Work16 >> 4) & 0xF;
	uint8 W3 = (Work16 >> 8) & 0xF;
	uint8 W4 = (Work16 >> 12) & 0xF;

	A1 -= W1 + !CHECKCARRY ();
	A2 -= W2;
	A3 -= W3;
	A4 -= W4;
	if (A1 > 9)
	{
	    A1 += 10;
	    A2--;
	}
	if (A2 > 9)
	{
	    A2 += 10;
	    A3--;
	}
	if (A3 > 9)
	{
	    A3 += 10;
	    A4--;
	}
	if (A4 > 9)
	{
	    A4 += 10;
	    CLEARCARRY ();
	}
	else
	{
	    SETCARRY ();
	}

	uint16 Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
	if ((reg->A.W ^ Work16) &
	    (reg->A.W ^ Ans16) & 0x8000)
	    SETOVERFLOW();
	else
	    CLEAROVERFLOW();
	reg->A.W = Ans16;
	SETZN16 (reg->A.W);
    }
    else
    {

	int32 Int32 = (long) reg->A.W - (long) Work16 + (long) CHECKCARRY() - 1;

	icpu->_Carry = Int32 >= 0;

	if ((reg->A.W ^ Work16) &
	    (reg->A.W ^ (uint16) Int32) & 0x8000)
	    SETOVERFLOW();
	else
	    CLEAROVERFLOW ();
	reg->A.W = (uint16) Int32;
	SETZN16 (reg->A.W);
    }
}

STATIC inline void SBC8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    uint8 Work8 = S9xGetByte (OpAddress, cpu);
    if (CHECKDECIMAL())
    {
	uint8 A1 = (reg->A.W) & 0xF;
	uint8 A2 = (reg->A.W >> 4) & 0xF;
	uint8 W1 = Work8 & 0xF;
	uint8 W2 = (Work8 >> 4) & 0xF;

	A1 -= W1 + !CHECKCARRY ();
	A2 -= W2;
	if (A1 > 9)
	{
	    A1 += 10;
	    A2--;
	}
	if (A2 > 9)
	{
	    A2 += 10;
	    CLEARCARRY ();
	}
	else
	{
	    SETCARRY ();
	}

	uint8 Ans8 = (A2 << 4) | A1;
	if ((reg->AL ^ Work8) &
	    (reg->AL ^ Ans8) & 0x80)
	    SETOVERFLOW ();
	else
	    CLEAROVERFLOW ();
	reg->AL = Ans8;
	SETZN8 (reg->AL);
    }
    else
    {
	int32 Int32 = (short) reg->AL - (short) Work8 + (short) CHECKCARRY() - 1;

	icpu->_Carry = Int32 >= 0;
	if ((reg->AL ^ Work8) &
	    (reg->AL ^ (uint8) Int32) & 0x80)
	    SETOVERFLOW ();
	else
	    CLEAROVERFLOW ();
	reg->AL = (uint8) Int32;
	SETZN8 (reg->AL);
    }
}

STATIC inline void STA16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    S9xSetWord (reg->A.W, OpAddress, cpu);
}

STATIC inline void STA8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    S9xSetByte (reg->AL, OpAddress, cpu);
}

STATIC inline void STX16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    S9xSetWord (reg->X.W, OpAddress, cpu);
}

STATIC inline void STX8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    S9xSetByte (reg->XL, OpAddress, cpu);
}

STATIC inline void STY16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    S9xSetWord (reg->Y.W, OpAddress, cpu);
}

STATIC inline void STY8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    S9xSetByte (reg->YL, OpAddress, cpu);
}

STATIC inline void STZ16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    S9xSetWord (0, OpAddress, cpu);
}

STATIC inline void STZ8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    S9xSetByte (0, OpAddress, cpu);
}

STATIC inline void TSB16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress, cpu);
    icpu->_Zero = (Work16 & reg->A.W) != 0;
    Work16 |= reg->A.W;
    S9xSetWord (Work16, OpAddress, cpu);
}

STATIC inline void TSB8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress, cpu);
    icpu->_Zero = Work8 & reg->AL;
    Work8 |= reg->AL;
    S9xSetByte (Work8, OpAddress, cpu);
}

STATIC inline void TRB16 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress, cpu);
    icpu->_Zero = (Work16 & reg->A.W) != 0;
    Work16 &= ~reg->A.W;
    S9xSetWord (Work16, OpAddress, cpu);
}

STATIC inline void TRB8 (long OpAddress, struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef VAR_CYCLES
    cpu->Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress, cpu);
    icpu->_Zero = Work8 & reg->AL;
    Work8 &= ~reg->AL;
    S9xSetByte (Work8, OpAddress, cpu);
}
#endif
