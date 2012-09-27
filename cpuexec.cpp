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
#include <fcntl.h>
//#include <sys/mman.h>
#include "snes9x.h"
#include "memmap.h"
#include "cpuops.h"
#include "ppu.h"
#include "cpuexec.h"
#include "debug.h"
#include "snapshot.h"
#include "gfx.h"
#include "missing.h"
#include "apu.h"
#include "dma.h"
#include "fxemu.h"
#include "sa1.h"

void S9xMainLoop (void)
{
	struct SCPUState *cpu = &CPU;
	struct SICPU *icpu = &ICPU;
	struct SIAPU *iapu = &IAPU;
	struct SAPU *apu = &APU;
	struct SRegisters *reg = &Registers;
	struct SAPURegisters *areg = &APURegisters;

    for (;;)
    {
		APU_EXECUTE ();

		if (cpu->Flags)
		{
			if (cpu->Flags & NMI_FLAG)
			{
				if (--cpu->NMICycleCount == 0)
				{
					cpu->Flags &= ~NMI_FLAG;
					if (cpu->WaitingForInterrupt)
					{
						cpu->WaitingForInterrupt = FALSE;
						++cpu->PC;
					}
					S9xOpcode_NMI ();
				}
			}

#ifdef DEBUGGER
			if ((cpu->Flags & BREAK_FLAG) &&
			!(cpu->Flags & SINGLE_STEP_FLAG))
			{
				for (int Break = 0; Break != 6; Break++)
				{
					if (S9xBreakpoint[Break].Enabled &&
					S9xBreakpoint[Break].Bank == Registers.PB &&
					S9xBreakpoint[Break].Address == cpu->PC - cpu->PCBase)
					{
						if (S9xBreakpoint[Break].Enabled == 2)
							S9xBreakpoint[Break].Enabled = TRUE;
						else
							cpu->Flags |= DEBUG_MODE_FLAG;
					}
				}
			}
#endif
			CHECK_SOUND ();

			if (cpu->Flags & IRQ_PENDING_FLAG)
			{
				if (cpu->IRQCycleCount == 0)
				{
					if (cpu->WaitingForInterrupt)
					{
						cpu->WaitingForInterrupt = FALSE;
						cpu->PC++;
					}
					if (cpu->IRQActive)
//					if (cpu->IRQActive && !Settings.DisableIRQ)
					{
						if (!CHECKFLAG (IRQ))
							S9xOpcode_IRQ ();
					}
					else
						cpu->Flags &= ~IRQ_PENDING_FLAG;
				}
				else
					cpu->IRQCycleCount--;
			}
#ifdef DEBUGGER
			if (cpu->Flags & DEBUG_MODE_FLAG)
				break;
#endif
			if (cpu->Flags & SCAN_KEYS_FLAG)
				break;
#ifdef DEBUGGER
			if (cpu->Flags & TRACE_FLAG)
				S9xTrace ();

			if (cpu->Flags & SINGLE_STEP_FLAG)
			{
				cpu->Flags &= ~SINGLE_STEP_FLAG;
				cpu->Flags |= DEBUG_MODE_FLAG;
			}
#endif
		} //if (CPU.Flags)
#ifdef CPU_SHUTDOWN
		cpu->PCAtOpcodeStart = cpu->PC;
#endif
#ifdef VAR_CYCLES
		cpu->Cycles += cpu->MemSpeed;
#else
		cpu->Cycles += icpu->Speed [*cpu->PC];
#endif
		(*icpu->S9xOpcodes [*cpu->PC++].S9xOpcode) (reg, icpu, cpu);

		S9xUpdateAPUTimer();

		if (SA1.Executing)
			S9xSA1MainLoop ();

		DO_HBLANK_CHECK();
	}
    Registers.PC = cpu->PC - cpu->PCBase;
    S9xPackStatus ();
    areg->PC = iapu->PC - iapu->RAM;
    S9xAPUPackStatus_OP ();
    if (cpu->Flags & SCAN_KEYS_FLAG)
    {
#ifdef DEBUGGER
		if (!(cpu->Flags & FRAME_ADVANCE_FLAG))
#endif
			S9xSyncSpeed ();
		cpu->Flags &= ~SCAN_KEYS_FLAG;
    }
#ifdef DETECT_NASTY_FX_INTERLEAVE
    if (cpu->BRKTriggered && Settings.SuperFX && !cpu->TriedInterleavedMode2)
    {
		cpu->TriedInterleavedMode2 = TRUE;
		cpu->BRKTriggered = FALSE;
		S9xDeinterleaveMode2 ();
    }
#endif
}


void S9xSetIRQ (uint32 source, struct SCPUState *cpu)
{
    cpu->IRQActive |= source;
    cpu->Flags |= IRQ_PENDING_FLAG;
    cpu->IRQCycleCount = 3;
    if (cpu->WaitingForInterrupt)
    {
		// Force IRQ to trigger immediately after WAI - 
		// Final Fantasy Mystic Quest crashes without this.
		cpu->IRQCycleCount = 0;
		cpu->WaitingForInterrupt = FALSE;
		cpu->PC++;
    }
}

void S9xClearIRQ (uint32 source)
{
    CLEAR_IRQ_SOURCE (source);
}

void S9xDoHBlankProcessing (struct SCPUState *cpu, struct SAPU *apu, struct SIAPU *iapu)
{
	struct SPPU *ppu = &PPU;
	struct InternalPPU *ippu = &IPPU;

#ifdef CPU_SHUTDOWN
    cpu->WaitCounter++;
#endif
    switch (cpu->WhichEvent)
    {
    case HBLANK_START_EVENT:
		if (ippu->HDMA && cpu->V_Counter <= ppu->ScreenHeight)
			ippu->HDMA = S9xDoHDMA (ippu, ppu, cpu);
	break;

    case HBLANK_END_EVENT:

	S9xSuperFXExec ();

#ifndef STORM
	if (Settings.SoundSync)
	    S9xGenerateSound ();
#endif

	cpu->Cycles -= Settings.H_Max;
	IAPU.NextAPUTimerPos -= (Settings.H_Max * 10000L);
	if (iapu->APUExecuting)
	    apu->Cycles -= Settings.H_Max;
	else
	    apu->Cycles = 0;

	cpu->NextEvent = -1;
	ICPU.Scanline++;

	if (++cpu->V_Counter > (Settings.PAL ? SNES_MAX_PAL_VCOUNTER : SNES_MAX_NTSC_VCOUNTER))
	{
	    ppu->OAMAddr = ppu->SavedOAMAddr;
	    ppu->OAMFlip = 0;
	    cpu->V_Counter = 0;
	    cpu->NMIActive = FALSE;
	    ICPU.Frame++;
	    ppu->HVBeamCounterLatched = 0;
	    cpu->Flags |= SCAN_KEYS_FLAG;
	    S9xStartHDMA ();
	}

	if (ppu->VTimerEnabled && !ppu->HTimerEnabled &&
	    cpu->V_Counter == ppu->IRQVBeamPos)
	{
	    S9xSetIRQ (PPU_V_BEAM_IRQ_SOURCE, cpu);
	}

	if (cpu->V_Counter == ppu->ScreenHeight + FIRST_VISIBLE_LINE)
	{
	    // Start of V-blank
	    S9xEndScreenRefresh (ppu);
	    ppu->FirstSprite = 0;
	    ippu->HDMA = 0;
	    // Bits 7 and 6 of $4212 are computed when read in S9xGetPPU.
	    missing.dma_this_frame = 0;
	    ippu->MaxBrightness = ppu->Brightness;
	    ppu->ForcedBlanking = (Memory.FillRAM [0x2100] >> 7) & 1;

		if(!PPU.ForcedBlanking){
			PPU.OAMAddr = PPU.SavedOAMAddr;
			{
				uint8 tmp = 0;
				if(PPU.OAMPriorityRotation)
					tmp = (PPU.OAMAddr&0xFE)>>1;
				if((PPU.OAMFlip&1) || PPU.FirstSprite!=tmp){
					PPU.FirstSprite=tmp;
					IPPU.OBJChanged=TRUE;
				}
			}
			PPU.OAMFlip = 0;
		}

	    Memory.FillRAM[0x4210] = 0x80;

	    if (Memory.FillRAM[0x4200] & 0x80)
	    {
			cpu->NMIActive = TRUE;
			cpu->Flags |= NMI_FLAG;
			cpu->NMICycleCount = cpu->NMITriggerPoint;
	    }
    }

	if (cpu->V_Counter == ppu->ScreenHeight + 3)
	    S9xUpdateJoypads (&IPPU);

	if (cpu->V_Counter == FIRST_VISIBLE_LINE)
	{
	    Memory.FillRAM[0x4210] = 0;
	    cpu->Flags &= ~NMI_FLAG;
	    S9xStartScreenRefresh ();
	}
	if (cpu->V_Counter >= FIRST_VISIBLE_LINE &&
	    cpu->V_Counter < ppu->ScreenHeight + FIRST_VISIBLE_LINE)
	{
	    RenderLine (cpu->V_Counter - FIRST_VISIBLE_LINE, ppu);
	}
	// Use TimerErrorCounter to skip update of SPC700 timers once
	// every 128 updates. Needed because this section of code is called
	// once every emulated 63.5 microseconds, which coresponds to
	// 15.750KHz, but the SPC700 timers need to be updated at multiples
	// of 8KHz, hence the error correction.
//	IAPU.TimerErrorCounter++;
//	if (IAPU.TimerErrorCounter >= )
//	    IAPU.TimerErrorCounter = 0;
//	else
	{
	    if (apu->TimerEnabled [2])
	    {
			apu->Timer [2] += 4;
			while (apu->Timer [2] >= apu->TimerTarget [2])
			{
				iapu->RAM [0xff] = (iapu->RAM [0xff] + 1) & 0xf;
				apu->Timer [2] -= apu->TimerTarget [2];
#ifdef SPC700_SHUTDOWN		
				iapu->WaitCounter++;
				iapu->APUExecuting = TRUE;
#endif		
			}
	    }
	    if (cpu->V_Counter & 1)
	    {
			if (apu->TimerEnabled [0])
			{
				apu->Timer [0]++;
				if (apu->Timer [0] >= apu->TimerTarget [0])
				{
					iapu->RAM [0xfd] = (iapu->RAM [0xfd] + 1) & 0xf;
					apu->Timer [0] = 0;
#ifdef SPC700_SHUTDOWN		
					iapu->WaitCounter++;
					iapu->APUExecuting = TRUE;
#endif		    
			    }
			}
			if (apu->TimerEnabled [1])
			{
				apu->Timer [1]++;
				if (apu->Timer [1] >= apu->TimerTarget [1])
				{
					iapu->RAM [0xfe] = (iapu->RAM [0xfe] + 1) & 0xf;
					apu->Timer [1] = 0;
#ifdef SPC700_SHUTDOWN		
					iapu->WaitCounter++;
					iapu->APUExecuting = TRUE;
#endif		    
			    }
			}
	    }
	}
	break; //HBLANK_END_EVENT

    case HTIMER_BEFORE_EVENT:
    case HTIMER_AFTER_EVENT:
	if (ppu->HTimerEnabled &&
	    (!ppu->VTimerEnabled || cpu->V_Counter == ppu->IRQVBeamPos))
	{
	    S9xSetIRQ (PPU_H_BEAM_IRQ_SOURCE, cpu);
	}
	break;
    }
//    S9xReschedule ();
    uint8 which;
    long max;
    
    if (cpu->WhichEvent == HBLANK_START_EVENT ||
	cpu->WhichEvent == HTIMER_AFTER_EVENT)
    {
	which = HBLANK_END_EVENT;
	max = Settings.H_Max;
    }
    else
    {
	which = HBLANK_START_EVENT;
	max = Settings.HBlankStart;
    }

    if (ppu->HTimerEnabled &&
        (long) ppu->HTimerPosition < max &&
	(long) ppu->HTimerPosition > cpu->NextEvent &&
	(!ppu->VTimerEnabled ||
	 (ppu->VTimerEnabled && cpu->V_Counter == ppu->IRQVBeamPos)))
    {
	which = (long) ppu->HTimerPosition < Settings.HBlankStart ?
			HTIMER_BEFORE_EVENT : HTIMER_AFTER_EVENT;
	max = ppu->HTimerPosition;
    }
    cpu->NextEvent = max;
    cpu->WhichEvent = which;
}
