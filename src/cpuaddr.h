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
#ifndef _CPUADDR_H_
#define _CPUADDR_H_

// EXTERN_C long OpAddress;

STATIC inline long Immediate8(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = icpu->ShiftedPB + cpu->PC - cpu->PCBase;
	cpu->PC++;
	return OpAddress;
}

STATIC inline long Immediate16(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = icpu->ShiftedPB + cpu->PC - cpu->PCBase;
	cpu->PC += 2;
	return OpAddress;
}

STATIC inline long Relative(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	int8 Int8 = *cpu->PC++;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeed;
#endif
	return ((int)(cpu->PC - cpu->PCBase) + Int8) & 0xffff;
}

STATIC inline long RelativeLong(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = READ_WORD(cpu->PC);
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeedx2 + ONE_CYCLE;
#endif
	cpu->PC += 2;
	OpAddress += (cpu->PC - cpu->PCBase);
	OpAddress &= 0xffff;
	return OpAddress;
}

STATIC inline long AbsoluteIndexedIndirect(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = (reg->X.W + READ_WORD(CPU.PC)) & 0xffff;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeedx2;
#endif
	cpu->PC += 2;
	return S9xGetWord(icpu->ShiftedPB + OpAddress, cpu);
}

STATIC inline long AbsoluteIndirectLong(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = READ_WORD(cpu->PC);
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeedx2;
#endif
	cpu->PC += 2;
	return S9xGetWord(OpAddress, cpu) | (S9xGetByte(OpAddress + 2, cpu) << 16);
}

STATIC inline long AbsoluteIndirect(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = READ_WORD(cpu->PC);
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeedx2;
#endif
	cpu->PC += 2;
	return S9xGetWord(OpAddress, cpu) + icpu->ShiftedPB;
}

STATIC inline long Absolute(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = READ_WORD(cpu->PC) + icpu->ShiftedDB;
	cpu->PC += 2;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeedx2;
#endif
	return OpAddress;
}

STATIC inline long AbsoluteLong(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
#ifdef FAST_ALIGNED_LSB_WORD_ACCESS
	long OpAddress;
	if (((int32_t)cpu->PC & 1) == 0)
		OpAddress = (*(uint16_t *)cpu->PC) + (cpu->PC[2] << 16);
	else
		OpAddress = *CPU.PC + ((*(uint16_t *)(cpu->PC + 1)) << 8);
#else
	long OpAddress = READ_3WORD(cpu->PC);
#endif
	cpu->PC += 3;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeedx2 + cpu->MemSpeed;
#endif
	return OpAddress;
}

STATIC inline long Direct(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = (*cpu->PC++ + reg->D.W) & 0xffff;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeed;
#endif
	// if (reg->DL != 0) cpu->Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC inline long DirectIndirectIndexed(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = (*cpu->PC++ + reg->D.W) & 0xffff;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeed;
#endif

	OpAddress = icpu->ShiftedDB + S9xGetWord(OpAddress, cpu) + reg->Y.W;

	// if (reg->DL != 0) cpu->Cycles += ONE_CYCLE;
	// XXX: always add one if STA
	// XXX: else Add one cycle if crosses page boundary
	return OpAddress;
}

STATIC inline long DirectIndirectIndexedLong(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = (*cpu->PC++ + reg->D.W) & 0xffff;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeed;
#endif

	OpAddress = S9xGetWord(OpAddress, cpu) + (S9xGetByte(OpAddress + 2, cpu) << 16) + reg->Y.W;
	// if (reg->DL != 0) cpu->Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC inline long DirectIndexedIndirect(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = (*cpu->PC++ + reg->D.W + reg->X.W) & 0xffff;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeed;
#endif

	OpAddress = S9xGetWord(OpAddress, cpu) + icpu->ShiftedDB;

#ifdef VAR_CYCLES
	// if (reg->DL != 0)
	//	cpu->Cycles += TWO_CYCLES;
	// else
	cpu->Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC inline long DirectIndexedX(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = (*cpu->PC++ + reg->D.W + reg->X.W) & 0xffff;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeed;
#endif

#ifdef VAR_CYCLES
	// if (reg->DL != 0)
	//	cpu->Cycles += TWO_CYCLES;
	// else
	cpu->Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC inline long DirectIndexedY(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = (*cpu->PC++ + reg->D.W + reg->Y.W) & 0xffff;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeed;
#endif

#ifdef VAR_CYCLES
	// if (reg->DL != 0)
	//	cpu->Cycles += TWO_CYCLES;
	// else
	cpu->Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC inline long AbsoluteIndexedX(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = icpu->ShiftedDB + READ_WORD(cpu->PC) + reg->X.W;
	cpu->PC += 2;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeedx2;
#endif
	// XXX: always add one cycle for ROL, LSR, etc
	// XXX: else is cross page boundary add one cycle
	return OpAddress;
}

STATIC inline long AbsoluteIndexedY(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = icpu->ShiftedDB + READ_WORD(cpu->PC) + reg->Y.W;
	cpu->PC += 2;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeedx2;
#endif
	// XXX: always add cycle for STA
	// XXX: else is cross page boundary add one cycle
	return OpAddress;
}

STATIC inline long AbsoluteLongIndexedX(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
#ifdef FAST_ALIGNED_LSB_WORD_ACCESS
	long OpAddress;
	if (((int32_t)cpu->PC & 1) == 0)
		OpAddress = ((*(uint16_t *)cpu->PC) + (cpu->PC[2] << 16) + reg->X.W) & 0xFFFFFF;
	else
		OpAddress = (*cpu->PC + ((*(uint16_t *)(cpu->PC + 1)) << 8) + reg->X.W) & 0xFFFFFF;
#else
	long OpAddress = (READ_3WORD(cpu->PC) + reg->X.W) & 0x00ffffff;
#endif
	cpu->PC += 3;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeedx2 + cpu->MemSpeed;
#endif
	return OpAddress;
}

STATIC inline long DirectIndirect(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = (*cpu->PC++ + reg->D.W) & 0xffff;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeed;
#endif
	OpAddress = S9xGetWord(OpAddress, cpu) + icpu->ShiftedDB;

	// if (reg->DL != 0) cpu->Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC inline long DirectIndirectLong(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = (*cpu->PC++ + reg->D.W) & 0xffff;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeed;
#endif
	OpAddress = S9xGetWord(OpAddress, cpu) + (S9xGetByte(OpAddress + 2, cpu) << 16);
	// if (reg->DL != 0) cpu->Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC inline long StackRelative(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = (*cpu->PC++ + reg->S.W) & 0xffff;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeed;
	cpu->Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC inline long StackRelativeIndirectIndexed(struct SRegisters *reg, struct SICPU *icpu, struct SCPUState *cpu)
{
	long OpAddress = (*cpu->PC++ + reg->S.W) & 0xffff;
#ifdef VAR_CYCLES
	cpu->Cycles += cpu->MemSpeed;
	cpu->Cycles += TWO_CYCLES;
#endif
	OpAddress = (S9xGetWord(OpAddress, cpu) + icpu->ShiftedDB + reg->Y.W) & 0xffffff;
	return OpAddress;
}
#endif
