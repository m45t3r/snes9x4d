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

#ifndef _GETSET_H_
#define _GETSET_H_

#include "ppu.h"
#include "dsp1.h"
#include "cpuexec.h"
#include "sa1.h"
#include "spc7110.h"
#include "obc1.h"

INLINE uint8 S9xGetByte(uint32 Address, struct SCPUState *cpu)
{
#if defined(VAR_CYCLES) || defined(CPU_SHUTDOWN)
	int block;
	uint8 *GetAddress = Memory.Map[block = (Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#else
	uint8 *GetAddress = Memory.Map[(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#endif
	if (GetAddress >= (uint8 *)CMemory::MAP_LAST) {
#ifdef VAR_CYCLES
		cpu->Cycles += Memory.MemorySpeed[block];
#endif
#ifdef CPU_SHUTDOWN
		if (Memory.BlockIsRAM[block])
			cpu->WaitAddress = cpu->PCAtOpcodeStart;
#endif
		return (*(GetAddress + (Address & 0xffff)));
	}

	switch ((intptr_t)GetAddress) {
	case CMemory::MAP_PPU:
#ifdef VAR_CYCLES
		if (!cpu->InDMA)
			cpu->Cycles += ONE_CYCLE;
#endif
		return (S9xGetPPU(Address & 0xffff, &PPU, &Memory));
	case CMemory::MAP_CPU:
#ifdef VAR_CYCLES
		cpu->Cycles += ONE_CYCLE;
#endif
		return (S9xGetCPU(Address & 0xffff, &IPPU, &Memory));
	case CMemory::MAP_DSP:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif

#ifdef DSP_DUMMY_LOOPS
		printf("Get DSP Byte @ %06X\n", Address);
#endif
		return (S9xGetDSP(Address & 0xffff));
	case CMemory::MAP_SA1RAM:
	case CMemory::MAP_LOROM_SRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif
		// Address &0x7FFF -offset into bank
		// Address&0xFF0000 -bank
		// bank>>1 | offset = s-ram address, unbound
		// unbound & SRAMMask = Sram offset
		return (*(Memory.SRAM + ((((Address & 0xFF0000) >> 1) | (Address & 0x7FFF)) & Memory.SRAMMask)));
		// return (*(Memory.SRAM + ((Address & Memory.SRAMMask))));

		//	case CMemory::MAP_RONLY_SRAM:
	case CMemory::MAP_HIROM_SRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif
		return (
		    *(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)));
	case CMemory::MAP_BWRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif
		return (*(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)));
	case CMemory::MAP_C4:
		return (S9xGetC4(Address & 0xffff));

#ifndef _ZAURUS
	case CMemory::MAP_SPC7110_ROM:
#ifdef SPC7110_DEBUG
		printf("reading spc7110 ROM (byte) at %06X\n", Address);
#endif
		cpu->Cycles += SLOW_ONE_CYCLE;
		return S9xGetSPC7110Byte(Address);

	case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
		printf("reading Bank 50 (byte)\n");
#endif
		cpu->Cycles += SLOW_ONE_CYCLE;
		return S9xGetSPC7110(0x4800);
#endif
	case CMemory::MAP_OBC_RAM:
		cpu->Cycles += SLOW_ONE_CYCLE;
		return GetOBC1(Address & 0xffff);

#ifndef _ZAURUS
	case CMemory::MAP_SETA_DSP:
		cpu->Cycles += SLOW_ONE_CYCLE;
		return S9xGetSetaDSP(Address);

	case CMemory::MAP_SETA_RISC:
		cpu->Cycles += SLOW_ONE_CYCLE;
		return S9xGetST018(Address);
#endif

	case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
		printf("DEBUG R(B) %06x\n", Address);
#endif
		// return OpenBus;
		return ((Address >> 8) & 0xff);

	default:
	case CMemory::MAP_NONE:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif

#ifdef MK_TRACE_BAD_READS
		char address[20];
		sprintf(address, "%06X", Address);
		MessageBox(GUI.hWnd, address, "GetByte", MB_OK);
#endif

#ifdef DEBUGGER
		printf("R(B) %06x\n", Address);
#endif
		// return OpenBus;
		return ((Address >> 8) & 0xff);
	}
}

INLINE uint16 S9xGetWord(uint32 Address, struct SCPUState *cpu)
{
	if ((Address & 0x0fff) == 0x0fff) {
		// OpenBus = S9xGetByte(Address, cpu);
		// return (OpenBus | (S9xGetByte(Address + 1, cpu) << 8));
		return (S9xGetByte(Address, cpu) | (S9xGetByte(Address + 1, cpu) << 8));
	}
#if defined(VAR_CYCLES) || defined(CPU_SHUTDOWN)
	int block;
	uint8 *GetAddress = Memory.Map[block = (Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#else
	uint8 *GetAddress = Memory.Map[(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#endif
	if (GetAddress >= (uint8 *)CMemory::MAP_LAST) {
#ifdef VAR_CYCLES
		cpu->Cycles += Memory.MemorySpeed[block] << 1;
#endif
#ifdef CPU_SHUTDOWN
		if (Memory.BlockIsRAM[block])
			cpu->WaitAddress = cpu->PCAtOpcodeStart;
#endif
		return READ_WORD(GetAddress + (Address & 0xffff));
	}

	switch ((intptr_t)GetAddress) {
	case CMemory::MAP_PPU:
#ifdef VAR_CYCLES
		if (!cpu->InDMA)
			cpu->Cycles += TWO_CYCLES;
#endif
		return (S9xGetPPU(Address & 0xffff, &PPU, &Memory) |
			(S9xGetPPU((Address + 1) & 0xffff, &PPU, &Memory) << 8));
	case CMemory::MAP_CPU:
#ifdef VAR_CYCLES
		cpu->Cycles += TWO_CYCLES;
#endif
		return (S9xGetCPU(Address & 0xffff, &IPPU, &Memory) |
			(S9xGetCPU((Address + 1) & 0xffff, &IPPU, &Memory) << 8));
	case CMemory::MAP_DSP:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
#endif

#ifdef DSP_DUMMY_LOOPS
		printf("Get DSP Word @ %06X\n", Address);
#endif
		return (S9xGetDSP(Address & 0xffff) | (S9xGetDSP((Address + 1) & 0xffff) << 8));
	case CMemory::MAP_SA1RAM:
	case CMemory::MAP_LOROM_SRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
#endif

		// Address &0x7FFF -offset into bank
		// Address&0xFF0000 -bank
		// bank>>1 | offset = s-ram address, unbound
		// unbound & SRAMMask = Sram offset
		return (*(Memory.SRAM + ((((Address & 0xFF0000) >> 1) | (Address & 0x7FFF)) & Memory.SRAMMask))) |
		       ((*(Memory.SRAM +
			   (((((Address + 1) & 0xFF0000) >> 1) | ((Address + 1) & 0x7FFF)) & Memory.SRAMMask)))
			<< 8);

		// return (*(uint16 *)(Memory.SRAM +
		// 		    ((((Address & 0xFF0000) >> 1) | (Address & 0x7FFF)) & Memory.SRAMMask)); // |
		// 	(*(Memory.SRAM + ((Address + 1) & Memory.SRAMMask)) << 8));

	// case CMemory::MAP_RONLY_SRAM:
	case CMemory::MAP_HIROM_SRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
#endif
		return (
		    *(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)) |
		    (*(Memory.SRAM +
		       ((((Address + 1) & 0x7fff) - 0x6000 + (((Address + 1) & 0xf0000) >> 3)) & Memory.SRAMMask))
		     << 8));
	case CMemory::MAP_BWRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
#endif
		return (*(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)) |
			(*(Memory.BWRAM + (((Address + 1) & 0x7fff) - 0x6000)) << 8));
	case CMemory::MAP_C4:
		return (S9xGetC4(Address & 0xffff) | (S9xGetC4((Address + 1) & 0xffff) << 8));

#ifndef _ZAURUS
	case CMemory::MAP_SPC7110_ROM:
#ifdef SPC7110_DEBUG
		printf("reading spc7110 ROM (word) at %06X\n", Address);
#endif
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
		return (S9xGetSPC7110Byte(Address) | (S9xGetSPC7110Byte(Address + 1)) << 8);
	case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
		printf("reading Bank 50 (word)\n");
#endif
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
		return (S9xGetSPC7110(0x4800) | (S9xGetSPC7110(0x4800) << 8));
#endif
	case CMemory::MAP_OBC_RAM:
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
		return GetOBC1(Address & 0xFFFF) | GetOBC1((Address + 1) & 0xFFFF);

#ifndef _ZAURUS
	case CMemory::MAP_SETA_DSP:
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
		return S9xGetSetaDSP(Address) | (S9xGetSetaDSP((Address + 1)) << 8);

	case CMemory::MAP_SETA_RISC:
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
		return S9xGetST018(Address) | S9xGetST018((Address + 1));
#endif

	case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
		printf("DEBUG R(W) %06x\n", Address);
#endif
		// return (OpenBus | (OpenBus<<8));
		return (((Address >> 8) | (Address & 0xff00)) & 0xffff);

	default:
	case CMemory::MAP_NONE:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
#endif

#ifdef MK_TRACE_BAD_READS
		char address[20];
		sprintf(address, "%06X", Address);
		MessageBox(GUI.hWnd, address, "GetWord", MB_OK);
#endif

#ifdef DEBUGGER
		printf("R(W) %06x\n", Address);
#endif
		// return (OpenBus | (OpenBus<<8));
		return (((Address >> 8) | (Address & 0xff00)) & 0xffff);
	}
}

INLINE void S9xSetByte(uint8 Byte, uint32 Address, struct SCPUState *cpu)
{
#if defined(CPU_SHUTDOWN)
	cpu->WaitAddress = NULL;
#endif
#if defined(VAR_CYCLES)
	int block;
	uint8 *SetAddress = Memory.WriteMap[block = ((Address >> MEMMAP_SHIFT) & MEMMAP_MASK)];
#else
	uint8 *SetAddress = Memory.WriteMap[(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#endif

	if (SetAddress >= (uint8 *)CMemory::MAP_LAST) {
#ifdef VAR_CYCLES
		cpu->Cycles += Memory.MemorySpeed[block];
#endif
#ifdef CPU_SHUTDOWN
		SetAddress += Address & 0xffff;

		if (SetAddress == SA1.WaitByteAddress1 || SetAddress == SA1.WaitByteAddress2) {
			SA1.Executing = SA1ICPU.S9xOpcodes != NULL;
			SA1.WaitCounter = 0;
		}

		*SetAddress = Byte;
#else
		*(SetAddress + (Address & 0xffff)) = Byte;
#endif
		return;
	}

	switch ((intptr_t)SetAddress) {
	case CMemory::MAP_PPU:
#ifdef VAR_CYCLES
		if (!cpu->InDMA)
			cpu->Cycles += ONE_CYCLE;
#endif
		S9xSetPPU(Byte, Address & 0xffff, &PPU, &IPPU);
		return;

	case CMemory::MAP_CPU:
#ifdef VAR_CYCLES
		cpu->Cycles += ONE_CYCLE;
#endif
		S9xSetCPU(Byte, Address & 0xffff, &PPU, cpu);
		return;

	case CMemory::MAP_DSP:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif

#ifdef DSP_DUMMY_LOOPS
		printf("DSP Byte: %02X to %06X\n", Byte, Address);
#endif
		S9xSetDSP(Byte, Address & 0xffff);
		return;

	case CMemory::MAP_LOROM_SRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif
		if (Memory.SRAMMask) {
			*(Memory.SRAM + ((((Address & 0xFF0000) >> 1) | (Address & 0x7FFF)) & Memory.SRAMMask)) = Byte;
			// *(Memory.SRAM + (Address & Memory.SRAMMask)) = Byte;
			cpu->SRAMModified = TRUE;
		}
		return;

	case CMemory::MAP_HIROM_SRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif
		if (Memory.SRAMMask) {
			*(Memory.SRAM +
			  (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)) = Byte;
			cpu->SRAMModified = TRUE;
		}
		return;
	case CMemory::MAP_BWRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif
		*(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)) = Byte;
		cpu->SRAMModified = TRUE;
		return;

	case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
		printf("W(B) %06x\n", Address);
#endif

	case CMemory::MAP_SA1RAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif
		*(Memory.SRAM + (Address & 0xffff)) = Byte;
		SA1.Executing = !SA1.Waiting;
		break;

	case CMemory::MAP_C4:
		S9xSetC4(Byte, Address & 0xffff);
		return;

#ifndef _ZAURUS
	case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
		printf("Writing Byte at %06X\n", Address);
#endif
		cpu->Cycles += SLOW_ONE_CYCLE;
		s7r.bank50[(Address & 0xffff)] = (uint8)Byte;
		break;
#endif

	case CMemory::MAP_OBC_RAM:
		cpu->Cycles += SLOW_ONE_CYCLE;
		SetOBC1(Byte, Address & 0xFFFF);
		return;

#ifndef _ZAURUS
	case CMemory::MAP_SETA_DSP:
		cpu->Cycles += SLOW_ONE_CYCLE;
		S9xSetSetaDSP(Address, Byte);
		return;

	case CMemory::MAP_SETA_RISC:
		cpu->Cycles += SLOW_ONE_CYCLE;
		S9xSetST018(Address, Byte);
		return;
#endif

	default:
	case CMemory::MAP_NONE:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif

#ifdef MK_TRACE_BAD_WRITES
		char address[20];
		sprintf(address, "%06X", Address);
		MessageBox(GUI.hWnd, address, "SetByte", MB_OK);
#endif

#ifdef DEBUGGER
		printf("W(B) %06x\n", Address);
#endif
		return;
	}
}

INLINE void S9xSetWord(uint16 Word, uint32 Address, struct SCPUState *cpu)
{
	if ((Address & 0x0FFF) == 0x0FFF) {
		S9xSetByte(Word & 0x00FF, Address, cpu);
		S9xSetByte(Word >> 8, Address + 1, cpu);
		return;
	}

#if defined(CPU_SHUTDOWN)
	cpu->WaitAddress = NULL;
#endif
#if defined(VAR_CYCLES)
	int block;
	uint8 *SetAddress = Memory.WriteMap[block = ((Address >> MEMMAP_SHIFT) & MEMMAP_MASK)];
#else
	uint8 *SetAddress = Memory.WriteMap[(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#endif

	if (SetAddress >= (uint8 *)CMemory::MAP_LAST) {
#ifdef VAR_CYCLES
		cpu->Cycles += Memory.MemorySpeed[block] << 1;
#endif
#ifdef CPU_SHUTDOWN
		SetAddress += Address & 0xffff;

		if (SetAddress == SA1.WaitByteAddress1 || SetAddress == SA1.WaitByteAddress2) {
			SA1.Executing = SA1ICPU.S9xOpcodes != NULL;
			SA1.WaitCounter = 0;
		}

		WRITE_WORD(SetAddress, Word);
#else
		WRITE_WORD(SetAddress + (Address & 0xffff), Word);
#endif
		return;
	}

	switch ((intptr_t)SetAddress) {
	case CMemory::MAP_PPU:
#ifdef VAR_CYCLES
		if (!cpu->InDMA)
			cpu->Cycles += TWO_CYCLES;
#endif
		S9xSetPPU((uint8)Word, Address & 0xffff, &PPU, &IPPU);
		S9xSetPPU(Word >> 8, (Address & 0xffff) + 1, &PPU, &IPPU);
		return;

	case CMemory::MAP_CPU:
#ifdef VAR_CYCLES
		cpu->Cycles += TWO_CYCLES;
#endif
		S9xSetCPU((uint8)Word, (Address & 0xffff), &PPU, cpu);
		S9xSetCPU(Word >> 8, (Address & 0xffff) + 1, &PPU, cpu);
		return;

	case CMemory::MAP_DSP:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
#endif

#ifdef DSP_DUMMY_LOOPS
		printf("DSP Word: %04X to %06X\n", Word, Address);
#endif
		S9xSetDSP((uint8)Word, (Address & 0xffff));
		S9xSetDSP(Word >> 8, (Address & 0xffff) + 1);
		return;

	case CMemory::MAP_LOROM_SRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
#endif
		if (Memory.SRAMMask) {
			*(Memory.SRAM + ((((Address & 0xFF0000) >> 1) | (Address & 0x7FFF)) & Memory.SRAMMask)) =
			    (uint8)Word;
			*(Memory.SRAM + (((((Address + 1) & 0xFF0000) >> 1) | ((Address + 1) & 0x7FFF)) &
					 Memory.SRAMMask)) = Word >> 8;

			// *(Memory.SRAM + (Address & Memory.SRAMMask)) = (uint8)Word;
			// *(Memory.SRAM + ((Address + 1) & Memory.SRAMMask)) = Word >> 8;
			cpu->SRAMModified = TRUE;
		}
		return;

	case CMemory::MAP_HIROM_SRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
#endif
		if (Memory.SRAMMask) {
			*(Memory.SRAM + ((((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) &
					  Memory.SRAMMask))) = (uint8)Word;
			*(Memory.SRAM + (((((Address + 1) & 0x7fff) - 0x6000 + (((Address + 1) & 0xf0000) >> 3)) &
					  Memory.SRAMMask))) = (uint8)(Word >> 8);
			cpu->SRAMModified = TRUE;
		}
		return;
	case CMemory::MAP_BWRAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
#endif
		*(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)) = (uint8)Word;
		*(Memory.BWRAM + (((Address + 1) & 0x7fff) - 0x6000)) = (uint8)(Word >> 8);
		cpu->SRAMModified = TRUE;
		return;

	case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
		printf("W(W) %06x\n", Address);
#endif
#ifndef _ZAURUS
	case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
		printf("Writing Word at %06X\n", Address);
#endif
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
		s7r.bank50[(Address & 0xffff)] = (uint8)Word;
		s7r.bank50[((Address + 1) & 0xffff)] = (uint8)Word;
		break;
#endif
	case CMemory::MAP_SA1RAM:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE;
#endif
		*(Memory.SRAM + (Address & 0xffff)) = (uint8)Word;
		*(Memory.SRAM + ((Address + 1) & 0xffff)) = (uint8)(Word >> 8);
		SA1.Executing = !SA1.Waiting;
		break;

	case CMemory::MAP_C4:
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
		S9xSetC4(Word & 0xff, Address & 0xffff);
		S9xSetC4((uint8)(Word >> 8), (Address + 1) & 0xffff);
		return;

	case CMemory::MAP_OBC_RAM:
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
		SetOBC1(Word & 0xff, Address & 0xFFFF);
		SetOBC1((uint8)(Word >> 8), (Address + 1) & 0xffff);
		return;

#ifndef _ZAURUS
	case CMemory::MAP_SETA_DSP:
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
		S9xSetSetaDSP(Address, Word & 0xff);
		S9xSetSetaDSP((Address + 1), (uint8)(Word >> 8));
		return;

	case CMemory::MAP_SETA_RISC:
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
		S9xSetST018(Address, Word & 0xff);
		S9xSetST018((Address + 1), (uint8)(Word >> 8));
		return;
#endif

	default:
	case CMemory::MAP_NONE:
#ifdef VAR_CYCLES
		cpu->Cycles += SLOW_ONE_CYCLE * 2;
#endif

#ifdef MK_TRACE_BAD_WRITES
		char address[20];
		sprintf(address, "%06X", Address);
		MessageBox(GUI.hWnd, address, "SetWord", MB_OK);
#endif

#ifdef DEBUGGER
		printf("W(W) %06x\n", Address);
#endif
		return;
	}
}

INLINE uint8 *GetBasePointer(uint32 Address)
{
	uint8 *GetAddress = Memory.Map[(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
	if (GetAddress >= (uint8 *)CMemory::MAP_LAST)
		return (GetAddress);
#ifndef _ZAURUS
	if (Settings.SPC7110 && ((Address & 0x7FFFFF) == 0x4800)) {
		return s7r.bank50;
	}
#endif
	switch ((intptr_t)GetAddress) {
#ifndef _ZAURUS
	case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
		printf("Getting Base pointer to DRAM\n");
#endif
		{
			return s7r.bank50;
		}
	case CMemory::MAP_SPC7110_ROM:
#ifdef SPC7110_DEBUG
		printf("Getting Base pointer to SPC7110ROM\n");
#endif
		return Get7110BasePtr(Address);
#endif
	case CMemory::MAP_PPU:
		// just a guess, but it looks like this should match the CPU as
		// a source.
		return (Memory.FillRAM);
		// return (Memory.FillRAM - 0x2000);
	case CMemory::MAP_CPU:
		// fixes Ogre Battle's green lines
		return (Memory.FillRAM);
		// return (Memory.FillRAM - 0x4000);
	case CMemory::MAP_DSP:
		return (Memory.FillRAM - 0x6000);
	case CMemory::MAP_SA1RAM:
	case CMemory::MAP_LOROM_SRAM:
		return (Memory.SRAM);
	case CMemory::MAP_HIROM_SRAM:
		return (Memory.SRAM - 0x6000);
	case CMemory::MAP_BWRAM:
		return (Memory.BWRAM - 0x6000);
	case CMemory::MAP_C4:
		return (Memory.C4RAM - 0x6000);
	case CMemory::MAP_OBC_RAM:
		return GetBasePointerOBC1(Address);

#ifndef _ZAURUS
	case CMemory::MAP_SETA_DSP:
		return Memory.SRAM;
#endif

	case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
		printf("GBP %06x\n", Address);
#endif
	default:
	case CMemory::MAP_NONE:
#ifdef DEBUGGER
		printf("GBP %06x\n", Address);
#endif
		return (0);
	}
}

INLINE uint8 *S9xGetMemPointer(uint32 Address)
{
	uint8 *GetAddress = Memory.Map[(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
	if (GetAddress >= (uint8 *)CMemory::MAP_LAST)
		return (GetAddress + (Address & 0xffff));
#ifndef _ZAURUS
	if (Settings.SPC7110 && ((Address & 0x7FFFFF) == 0x4800))
		return s7r.bank50;
#endif
	switch ((intptr_t)GetAddress) {
#ifndef _ZAURUS
	case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
		printf("Getting Base pointer to DRAM\n");
#endif
		return &s7r.bank50[Address & 0x0000FFFF];
#endif
	case CMemory::MAP_PPU:
		return (Memory.FillRAM - 0x2000 + (Address & 0xffff));
	case CMemory::MAP_CPU:
		return (Memory.FillRAM - 0x4000 + (Address & 0xffff));
	case CMemory::MAP_DSP:
		return (Memory.FillRAM - 0x6000 + (Address & 0xffff));
	case CMemory::MAP_SA1RAM:
	case CMemory::MAP_LOROM_SRAM:
		return (Memory.SRAM + (Address & 0xffff));
	case CMemory::MAP_HIROM_SRAM:
		return (Memory.SRAM - 0x6000 + (Address & 0xffff));
	case CMemory::MAP_BWRAM:
		return (Memory.BWRAM - 0x6000 + (Address & 0xffff));
	case CMemory::MAP_C4:
		return (Memory.C4RAM - 0x6000 + (Address & 0xffff));
	case CMemory::MAP_OBC_RAM:
		return GetMemPointerOBC1(Address);

#ifndef _ZAURUS
	case CMemory::MAP_SETA_DSP:
		return Memory.SRAM;
#endif

	case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
		printf("GMP %06x\n", Address);
#endif
	default:
	case CMemory::MAP_NONE:
#ifdef DEBUGGER
		printf("GMP %06x\n", Address);
#endif
		return (0);
	}
}

INLINE void S9xSetPCBase(uint32 Address, struct SCPUState *cpu)
{
#ifdef VAR_CYCLES
	int block;
	uint8 *GetAddress = Memory.Map[block = (Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#else
	uint8 *GetAddress = Memory.Map[(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#endif
	if (GetAddress >= (uint8 *)CMemory::MAP_LAST) {
#ifdef VAR_CYCLES
		cpu->MemSpeed = Memory.MemorySpeed[block];
		cpu->MemSpeedx2 = cpu->MemSpeed << 1;
#endif
		cpu->PCBase = GetAddress;
		cpu->PC = GetAddress + (Address & 0xffff);
		return;
	}

	switch ((intptr_t)GetAddress) {
	case CMemory::MAP_PPU:
#ifdef VAR_CYCLES
		cpu->MemSpeed = ONE_CYCLE;
		cpu->MemSpeedx2 = TWO_CYCLES;
#endif
		cpu->PCBase = Memory.FillRAM - 0x2000;
		cpu->PC = cpu->PCBase + (Address & 0xffff);
		return;

	case CMemory::MAP_CPU:
#ifdef VAR_CYCLES
		cpu->MemSpeed = ONE_CYCLE;
		cpu->MemSpeedx2 = TWO_CYCLES;
#endif
		cpu->PCBase = Memory.FillRAM - 0x4000;
		cpu->PC = cpu->PCBase + (Address & 0xffff);
		return;

	case CMemory::MAP_DSP:
#ifdef VAR_CYCLES
		cpu->MemSpeed = SLOW_ONE_CYCLE;
		cpu->MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif
		cpu->PCBase = Memory.FillRAM - 0x6000;
		cpu->PC = cpu->PCBase + (Address & 0xffff);
		return;

	case CMemory::MAP_SA1RAM:
	case CMemory::MAP_LOROM_SRAM:
#ifdef VAR_CYCLES
		cpu->MemSpeed = SLOW_ONE_CYCLE;
		cpu->MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif
		cpu->PCBase = Memory.SRAM;
		cpu->PC = cpu->PCBase + (Address & 0xffff);
		return;
	case CMemory::MAP_HIROM_SRAM:
#ifdef VAR_CYCLES
		cpu->MemSpeed = SLOW_ONE_CYCLE;
		cpu->MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif
		cpu->PCBase = Memory.SRAM - 0x6000;
		cpu->PC = cpu->PCBase + (Address & 0xffff);
		return;
	case CMemory::MAP_BWRAM:
#ifdef VAR_CYCLES
		cpu->MemSpeed = SLOW_ONE_CYCLE;
		cpu->MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif
		cpu->PCBase = Memory.BWRAM - 0x6000;
		cpu->PC = cpu->PCBase + (Address & 0xffff);
		return;
	case CMemory::MAP_C4:
#ifdef VAR_CYCLES
		cpu->MemSpeed = SLOW_ONE_CYCLE;
		cpu->MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif
		cpu->PCBase = Memory.C4RAM - 0x6000;
		cpu->PC = cpu->PCBase + (Address & 0xffff);
		return;
	case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
		printf("SBP %06x\n", Address);
#endif

	default:
	case CMemory::MAP_NONE:
#ifdef VAR_CYCLES
		cpu->MemSpeed = SLOW_ONE_CYCLE;
		cpu->MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif
#ifdef DEBUGGER
		printf("SBP %06x\n", Address);
#endif
		cpu->PCBase = Memory.SRAM;
		cpu->PC = Memory.SRAM + (Address & 0xffff);
		return;
	}
}
#endif
