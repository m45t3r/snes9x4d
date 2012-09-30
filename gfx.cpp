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

#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"
#include "display.h"
#include "gfx.h"
#include "apu.h"

#ifdef CHEATS
#include "cheats.h"
#endif

#define M7 19
#define M8 19

void ComputeClipWindows ();
//static void S9xDisplayFrameRate ();
//static void S9xDisplayString (const char *string);

extern uint8 BitShifts[8][4];
extern uint8 TileShifts[8][4];
extern uint8 PaletteShifts[8][4];
extern uint8 PaletteMasks[8][4];
extern uint8 Depths[8][4];
extern uint8 BGSizes [2];

extern NormalTileRenderer DrawTilePtr;
extern ClippedTileRenderer DrawClippedTilePtr;
extern NormalTileRenderer DrawHiResTilePtr;
extern ClippedTileRenderer DrawHiResClippedTilePtr;
extern LargePixelRenderer DrawLargePixelPtr;

extern struct SBG BG;

extern struct SLineData LineData[240];
extern struct SLineMatrixData LineMatrixData [240];

extern uint8  Mode7Depths [2];

extern bool8_32 Scale;

#define ON_MAIN(N) \
(GFX.r212c & (1 << (N)) && \
 !(PPU.BG_Forced & (1 << (N))))

#define SUB_OR_ADD(N) \
(GFX.r2131 & (1 << (N)))

#define ON_SUB(N) \
((GFX.r2130 & 0x30) != 0x30 && \
 (GFX.r2130 & 2) && \
 (GFX.r212d & (1 << N)) && \
 !(PPU.BG_Forced & (1 << (N))))

#define ANYTHING_ON_SUB \
((GFX.r2130 & 0x30) != 0x30 && \
 (GFX.r2130 & 2) && \
 (GFX.r212d & 0x1f))

#define ADD_OR_SUB_ON_ANYTHING \
(GFX.r2131 & 0x3f)

#define BLACK BUILD_PIXEL(0,0,0)

#ifndef _ZAURUS
void DrawTile (uint32 Tile, uint32 Offset, uint32 StartLine,
	       uint32 LineCount, struct SGFX * gfx);
void DrawClippedTile (uint32 Tile, uint32 Offset,
		      uint32 StartPixel, uint32 Width,
		      uint32 StartLine, uint32 LineCount, struct SGFX * gfx);
void DrawTilex2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		 uint32 LineCount, struct SGFX * gfx);
void DrawClippedTilex2 (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, struct SGFX * gfx);
void DrawTilex2x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
	       uint32 LineCount, struct SGFX * gfx);
void DrawClippedTilex2x2 (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Width,
			  uint32 StartLine, uint32 LineCount, struct SGFX * gfx);
void DrawLargePixel (uint32 Tile, uint32 Offset,
		     uint32 StartPixel, uint32 Pixels,
		     uint32 StartLine, uint32 LineCount, struct SGFX * gfx);
#endif
void DrawTile16 (uint32 Tile, uint32 Offset, uint32 StartLine,
	         uint32 LineCount, struct SGFX * gfx);
void DrawClippedTile16 (uint32 Tile, uint32 Offset,
		        uint32 StartPixel, uint32 Width,
		        uint32 StartLine, uint32 LineCount, struct SGFX * gfx);
void DrawTile16x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		   uint32 LineCount, struct SGFX * gfx);
void DrawClippedTile16x2 (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Width,
			  uint32 StartLine, uint32 LineCount, struct SGFX * gfx);
void DrawTile16x2x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		     uint32 LineCount, struct SGFX * gfx);
void DrawClippedTile16x2x2 (uint32 Tile, uint32 Offset,
			    uint32 StartPixel, uint32 Width,
			    uint32 StartLine, uint32 LineCount, struct SGFX * gfx);
void DrawLargePixel16 (uint32 Tile, uint32 Offset,
		       uint32 StartPixel, uint32 Pixels,
		       uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

void DrawTile16Add (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount, struct SGFX * gfx);

void DrawClippedTile16Add (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

void DrawTile16Add1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount, struct SGFX * gfx);

void DrawClippedTile16Add1_2 (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

void DrawTile16FixedAdd1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount, struct SGFX * gfx);

void DrawClippedTile16FixedAdd1_2 (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

void DrawTile16Sub (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount, struct SGFX * gfx);

void DrawClippedTile16Sub (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

void DrawTile16Sub1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount, struct SGFX * gfx);

void DrawClippedTile16Sub1_2 (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

void DrawTile16FixedSub1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount, struct SGFX * gfx);

void DrawClippedTile16FixedSub1_2 (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

void DrawLargePixel16Add (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

void DrawLargePixel16Add1_2 (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

void DrawLargePixel16Sub (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

void DrawLargePixel16Sub1_2 (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

bool8_32 S9xGraphicsInit ()
{
    register uint32 PixelOdd = 1;
    register uint32 PixelEven = 2;

#ifdef GFX_MULTI_FORMAT
    if (GFX.BuildPixel == NULL)
	S9xSetRenderPixelFormat (RGB565);
#endif

    for (uint8 bitshift = 0; bitshift < 4; bitshift++)
    {
	for (register char i = 0; i < 16; i++)
	{
	    register uint32 h = 0;
	    register uint32 l = 0;

#if defined(LSB_FIRST)
	    if (i & 8)
		h |= PixelOdd;
	    if (i & 4)
		h |= PixelOdd << 8;
	    if (i & 2)
		h |= PixelOdd << 16;
	    if (i & 1)
		h |= PixelOdd << 24;
	    if (i & 8)
		l |= PixelOdd;
	    if (i & 4)
		l |= PixelOdd << 8;
	    if (i & 2)
		l |= PixelOdd << 16;
	    if (i & 1)
		l |= PixelOdd << 24;
#else
	    if (i & 8)
		h |= (PixelOdd << 24);
	    if (i & 4)
		h |= (PixelOdd << 16);
	    if (i & 2)
		h |= (PixelOdd << 8);
	    if (i & 1)
		h |= PixelOdd;
	    if (i & 8)
		l |= (PixelOdd << 24);
	    if (i & 4)
		l |= (PixelOdd << 16);
	    if (i & 2)
		l |= (PixelOdd << 8);
	    if (i & 1)
		l |= PixelOdd;
#endif

	    odd_high[bitshift][i] = h;
	    odd_low[bitshift][i] = l;
	    h = l = 0;

#if defined(LSB_FIRST)
	    if (i & 8)
		h |= PixelEven;
	    if (i & 4)
		h |= PixelEven << 8;
	    if (i & 2)
		h |= PixelEven << 16;
	    if (i & 1)
		h |= PixelEven << 24;
	    if (i & 8)
		l |= PixelEven;
	    if (i & 4)
		l |= PixelEven << 8;
	    if (i & 2)
		l |= PixelEven << 16;
	    if (i & 1)
		l |= PixelEven << 24;
#else
	    if (i & 8)
		h |= (PixelEven << 24);
	    if (i & 4)
		h |= (PixelEven << 16);
	    if (i & 2)
		h |= (PixelEven << 8);
	    if (i & 1)
		h |= PixelEven;
	    if (i & 8)
		l |= (PixelEven << 24);
	    if (i & 4)
		l |= (PixelEven << 16);
	    if (i & 2)
		l |= (PixelEven << 8);
	    if (i & 1)
		l |= PixelEven;
#endif

	    even_high[bitshift][i] = h;
	    even_low[bitshift][i] = l;
	}
	PixelEven <<= 2;
	PixelOdd <<= 2;
    }

    GFX.RealPitch = GFX.Pitch2 = GFX.Pitch;
    GFX.ZPitch = GFX.Pitch;
    if (Settings.SixteenBit)
	GFX.ZPitch >>= 1;
    GFX.Delta = (GFX.SubScreen - GFX.Screen) >> 1;
    GFX.DepthDelta = GFX.SubZBuffer - GFX.ZBuffer;
    //GFX.InfoStringTimeout = 0;
    //GFX.InfoString = NULL;

    PPU.BG_Forced = 0;
    IPPU.OBJChanged = TRUE;
    if (Settings.Transparency)
	Settings.SixteenBit = TRUE;

    IPPU.DirectColourMapsNeedRebuild = TRUE;
    GFX.PixSize = 1;
#ifndef _ZAURUS
    if (Settings.SixteenBit)
    {
#endif
	DrawTilePtr = DrawTile16;
	DrawClippedTilePtr = DrawClippedTile16;
	DrawLargePixelPtr = DrawLargePixel16;
	DrawHiResTilePtr= DrawTile16;
	DrawHiResClippedTilePtr = DrawClippedTile16;
	GFX.PPL = GFX.Pitch >> 1;
	GFX.PPLx2 = GFX.Pitch;
#ifndef _ZAURUS
    }
    else
    {
	DrawTilePtr = DrawTile;
	DrawClippedTilePtr = DrawClippedTile;
	DrawLargePixelPtr = DrawLargePixel;
	DrawHiResTilePtr = DrawTile;
	DrawHiResClippedTilePtr = DrawClippedTile;
	GFX.PPL = GFX.Pitch;
	GFX.PPLx2 = GFX.Pitch * 2;
    }
#endif
    S9xFixColourBrightness ();

    if (Settings.SixteenBit)
    {
	if (!(GFX.X2 = (uint16 *) malloc (sizeof (uint16) * 0x10000)))
	    return (FALSE);

	if (!(GFX.ZERO_OR_X2 = (uint16 *) malloc (sizeof (uint16) * 0x10000)) ||
	    !(GFX.ZERO = (uint16 *) malloc (sizeof (uint16) * 0x10000)))
	{
	    if (GFX.ZERO_OR_X2)
	    {
		free ((char *) GFX.ZERO_OR_X2);
		GFX.ZERO_OR_X2 = NULL;
	    }
	    if (GFX.X2)
	    {
		free ((char *) GFX.X2);
		GFX.X2 = NULL;
	    }
	    return (FALSE);
	}
	uint32 r, g, b;

	// Build a lookup table that multiplies a packed RGB value by 2 with
	// saturation.
	for (r = 0; r <= MAX_RED; r++)
	{
	    uint32 r2 = r << 1;
	    if (r2 > MAX_RED)
		r2 = MAX_RED;
	    for (g = 0; g <= MAX_GREEN; g++)
	    {
		uint32 g2 = g << 1;
		if (g2 > MAX_GREEN)
		    g2 = MAX_GREEN;
		for (b = 0; b <= MAX_BLUE; b++)
		{
		    uint32 b2 = b << 1;
		    if (b2 > MAX_BLUE)
			b2 = MAX_BLUE;
		    GFX.X2 [BUILD_PIXEL2 (r, g, b)] = BUILD_PIXEL2 (r2, g2, b2);
		    GFX.X2 [BUILD_PIXEL2 (r, g, b) & ~ALPHA_BITS_MASK] = BUILD_PIXEL2 (r2, g2, b2);
		}
	    }
	}
	ZeroMemory (GFX.ZERO, 0x10000 * sizeof (uint16));
	ZeroMemory (GFX.ZERO_OR_X2, 0x10000 * sizeof (uint16));
	// Build a lookup table that if the top bit of the color value is zero
	// then the value is zero, otherwise multiply the value by 2. Used by
	// the color subtraction code.

#if defined(OLD_COLOUR_BLENDING)
	for (r = 0; r <= MAX_RED; r++)
	{
	    uint32 r2 = r;
	    if ((r2 & 0x10) == 0)
		r2 = 0;
	    else
		r2 = (r2 << 1) & MAX_RED;

	    for (g = 0; g <= MAX_GREEN; g++)
	    {
		uint32 g2 = g;
		if ((g2 & GREEN_HI_BIT) == 0)
		    g2 = 0;
		else
		    g2 = (g2 << 1) & MAX_GREEN;

		for (b = 0; b <= MAX_BLUE; b++)
		{
		    uint32 b2 = b;
		    if ((b2 & 0x10) == 0)
			b2 = 0;
		    else
			b2 = (b2 << 1) & MAX_BLUE;

		    GFX.ZERO_OR_X2 [BUILD_PIXEL2 (r, g, b)] = BUILD_PIXEL2 (r2, g2, b2);
		    GFX.ZERO_OR_X2 [BUILD_PIXEL2 (r, g, b) & ~ALPHA_BITS_MASK] = BUILD_PIXEL2 (r2, g2, b2);
		}
	    }
	}
#else
        for (r = 0; r <= MAX_RED; r++)
        {
            uint32 r2 = r;
            if ((r2 & 0x10) == 0)
                r2 = 0;
            else
                r2 = (r2 << 1) & MAX_RED;

            if (r2 == 0)
                r2 = 1;
            for (g = 0; g <= MAX_GREEN; g++)
            {
                uint32 g2 = g;
                if ((g2 & GREEN_HI_BIT) == 0)
                    g2 = 0;
                else
                    g2 = (g2 << 1) & MAX_GREEN;

                if (g2 == 0)
                    g2 = 1;
                for (b = 0; b <= MAX_BLUE; b++)
                {
                    uint32 b2 = b;
                    if ((b2 & 0x10) == 0)
                        b2 = 0;
                    else
                        b2 = (b2 << 1) & MAX_BLUE;

                    if (b2 == 0)
                        b2 = 1;
                    GFX.ZERO_OR_X2 [BUILD_PIXEL2 (r, g, b)] = BUILD_PIXEL2 (r2, g2, b2);
                    GFX.ZERO_OR_X2 [BUILD_PIXEL2 (r, g, b) & ~ALPHA_BITS_MASK] = BUILD_PIXEL2 (r2, g2, b2);
                }
            }
        }
#endif

	// Build a lookup table that if the top bit of the color value is zero
	// then the value is zero, otherwise its just the value.
	for (r = 0; r <= MAX_RED; r++)
	{
	    uint32 r2 = r;
	    if ((r2 & 0x10) == 0)
		r2 = 0;
	    else
		r2 &= ~0x10;

	    for (g = 0; g <= MAX_GREEN; g++)
	    {
		uint32 g2 = g;
		if ((g2 & GREEN_HI_BIT) == 0)
		    g2 = 0;
		else
		    g2 &= ~GREEN_HI_BIT;
		for (b = 0; b <= MAX_BLUE; b++)
		{
		    uint32 b2 = b;
		    if ((b2 & 0x10) == 0)
			b2 = 0;
		    else
			b2 &= ~0x10;

		    GFX.ZERO [BUILD_PIXEL2 (r, g, b)] = BUILD_PIXEL2 (r2, g2, b2);
		    GFX.ZERO [BUILD_PIXEL2 (r, g, b) & ~ALPHA_BITS_MASK] = BUILD_PIXEL2 (r2, g2, b2);
		}
	    }
	}
    }
    else
    {
	GFX.X2 = NULL;
	GFX.ZERO_OR_X2 = NULL;
	GFX.ZERO = NULL;
    }

    return (TRUE);
}

void S9xGraphicsDeinit (void)
{
    // Free any memory allocated in S9xGraphicsInit
    if (GFX.X2)
    {
	free ((char *) GFX.X2);
	GFX.X2 = NULL;
    }
    if (GFX.ZERO_OR_X2)
    {
	free ((char *) GFX.ZERO_OR_X2);
	GFX.ZERO_OR_X2 = NULL;
    }
    if (GFX.ZERO)
    {
	free ((char *) GFX.ZERO);
	GFX.ZERO = NULL;
    }
}

void S9xBuildDirectColourMaps ()
{
    for (register uint32 p = 0; p < 8; p++)
    {
	for (register uint32 c = 0; c < 256; c++)
	{
// XXX: Brightness
		 DirectColourMaps [p][c] = ((int)(((c & 7) << 2) | ((p & 1) << 1) << 11)) | 
								   ((int)(((c & 0x38) >> 1) | (p & 2) << 6)) |
									(int)(((c & 0xc0) >> 3) | (p & 4));
//	    DirectColourMaps [p][c] = BUILD_PIXEL (((c & 7) << 2) | ((p & 1) << 1),
//						   ((c & 0x38) >> 1) | (p & 2),
//						   ((c & 0xc0) >> 3) | (p & 4));
	}
    }
    IPPU.DirectColourMapsNeedRebuild = FALSE;
}

void S9xStartScreenRefresh ()
{
	struct SPPU * ppu = &PPU;
	struct InternalPPU *ippu = &IPPU;
	struct SGFX * gfx = &GFX; 

    if (gfx->InfoStringTimeout > 0 && --gfx->InfoStringTimeout == 0)
	gfx->InfoString = NULL;

    if (ippu->RenderThisFrame)
    {
#ifndef _ZAURUS
	if (!S9xInitUpdate ())
	{
	    ippu->RenderThisFrame = FALSE;
	    return;
	}
#endif
	ippu->RenderedFramesCount++;
	ippu->PreviousLine = ippu->CurrentLine = 0;
	ippu->MaxBrightness = ppu->Brightness;
	ippu->LatchedBlanking = ppu->ForcedBlanking;
	ippu->LatchedInterlace = (Memory.FillRAM[0x2133] & 1);
	if (Settings.SupportHiRes && (ppu->BGMode == 5 || ppu->BGMode == 6 ||
				      ippu->LatchedInterlace))
	{
	    if (ppu->BGMode == 5 || ppu->BGMode == 6)
	    {
		ippu->RenderedScreenWidth = 512;
		ippu->DoubleWidthPixels = TRUE;
	    }
	    if (ippu->LatchedInterlace)
	    {
		ippu->RenderedScreenHeight = ppu->ScreenHeight << 1;
		gfx->Pitch2 = gfx->RealPitch;
		gfx->Pitch = gfx->RealPitch * 2;
		if (Settings.SixteenBit)
		    gfx->PPL = gfx->PPLx2 = gfx->RealPitch;
		else
		    gfx->PPL = gfx->PPLx2 = gfx->RealPitch << 1;
	    }
	    else
	    {
		ippu->RenderedScreenHeight = ppu->ScreenHeight;
		gfx->Pitch2 = gfx->Pitch = gfx->RealPitch;
		if (Settings.SixteenBit)
		    gfx->PPL = gfx->Pitch >> 1;
		else
		    gfx->PPL = gfx->Pitch;
		gfx->PPLx2 = gfx->PPL << 1;
	    }
#if defined(USE_GLIDE) || defined(USE_OPENGL)
	    gfx->ZPitch = gfx->RealPitch;
	    if (Settings.SixteenBit)
		gfx->ZPitch >>= 1;
#endif
	}
	else
	{
	    ippu->RenderedScreenWidth = 256;
	    ippu->RenderedScreenHeight = ppu->ScreenHeight;
	    ippu->DoubleWidthPixels = FALSE;
#ifdef USE_GLIDE
	    if (Settings.GlideEnable)
	    {
		// Speed up hack for Glide: render low res. SNES images
		// into a handy 256x256 sized buffer that can be uploaded into
		// texture memory with one Glide call without having to copy it
		// into a second, suitably sized buffer first.
		gfx->Pitch2 = gfx->Pitch = 256 * sizeof (uint16);
		gfx->PPL = 256;
                gfx->PPLx2 = gfx->Pitch;
                gfx->ZPitch = 256;
	    }
	    else
#endif
#ifdef USE_OPENGL
            if (Settings.OpenGLEnable)
            {
                gfx->Pitch2 = gfx->Pitch = 256 * sizeof (uint16);
                gfx->PPL = 256;
                gfx->PPLx2 = gfx->Pitch;
                gfx->ZPitch = 256;
            }
            else
#endif
	    {
		if (Settings.SupportHiRes) {
			gfx->Pitch2 = gfx->Pitch = gfx->RealPitch;
			gfx->PPL = gfx->PPLx2 >> 1;
			gfx->ZPitch = gfx->RealPitch;
			if (Settings.SixteenBit)
			    gfx->ZPitch >>= 1;
		} else {
			gfx->Pitch2 = gfx->Pitch = GFX.Pitch;
			gfx->PPL = GFX.Pitch >> 1;
			gfx->PPLx2 = GFX.Pitch;
			gfx->ZPitch = GFX.Pitch >> 1;
		}
        }
	}
	ppu->RecomputeClipWindows = TRUE;
	gfx->DepthDelta = gfx->SubZBuffer - gfx->ZBuffer;
	gfx->Delta = (gfx->SubScreen - gfx->Screen) >> 1;
    }
    if (++ippu->FrameCount % Memory.ROMFramesPerSecond == 0)
    {
	ippu->DisplayedRenderedFrameCount = ippu->RenderedFramesCount;
	ippu->RenderedFramesCount = 0;
	ippu->FrameCount = 0;
    }
}

void RenderLine (uint8 C, struct SPPU *ppu)
{
	struct SLineData *ln = &LineData[C];

    if (IPPU.RenderThisFrame)
    {

	ln->BG[0].VOffset = ppu->BG[0].VOffset + 1;
	ln->BG[0].HOffset = ppu->BG[0].HOffset;
	ln->BG[1].VOffset = ppu->BG[1].VOffset + 1;
	ln->BG[1].HOffset = ppu->BG[1].HOffset;

	if (ppu->BGMode == 7)
	{
	    struct SLineMatrixData *p = &LineMatrixData [C];
	    p->MatrixA = ppu->MatrixA;
	    p->MatrixB = ppu->MatrixB;
	    p->MatrixC = ppu->MatrixC;
	    p->MatrixD = ppu->MatrixD;
	    p->CentreX = ppu->CentreX;
	    p->CentreY = ppu->CentreY;
	}
	else
	{
//#ifndef _ZAURUS
	    if (Settings.StarfoxHack && ppu->BG[2].VOffset == 0 &&
		ppu->BG[2].HOffset == 0xe000)
	    {
		ln->BG[2].VOffset = 0xe1;
		ln->BG[2].HOffset = 0;
	    }
	    else
//#endif
	    {
		ln->BG[2].VOffset = ppu->BG[2].VOffset + 1;
		ln->BG[2].HOffset = ppu->BG[2].HOffset;
		ln->BG[3].VOffset = ppu->BG[3].VOffset + 1;
		ln->BG[3].HOffset = ppu->BG[3].HOffset;
	    }

	}
	IPPU.CurrentLine = C + 1;
    }
}


void S9xEndScreenRefresh(struct SPPU *ppu)
{
	struct InternalPPU *ippu = &IPPU;
	struct SGFX *gfx = &GFX; 

    ippu->HDMAStarted = FALSE;

    if (ippu->RenderThisFrame)
    {
	FLUSH_REDRAW ();
	if (ippu->ColorsChanged)
	{
	    uint32 saved = ppu->CGDATA[0];
#ifndef _ZAURUS
	    if (!Settings.SixteenBit)
	    {
			// Hack for Super Mario World - to get its sky blue
			// (It uses Fixed colour addition on the backdrop colour)
			if (!(Memory.FillRAM [0x2131] & 0x80) &&
				(Memory.FillRAM[0x2131] & 0x20) &&
				(ppu->FixedColourRed || ppu->FixedColourGreen ||
				 ppu->FixedColourBlue))
			{
				ppu->CGDATA[0] = ppu->FixedColourRed |
						(ppu->FixedColourGreen << 5) |
						(ppu->FixedColourBlue << 10);
			}
	    }
#endif
	    ippu->ColorsChanged = FALSE;
		
	    S9xSetPalette ();

	    ppu->CGDATA[0] = saved;
	}
        if (
#ifdef USE_GLIDE
            !Settings.GlideEnable &&
#endif
#ifdef USE_OPENGL
            !Settings.OpenGLEnable &&
#endif
            TRUE)
        {
            gfx->Pitch = gfx->Pitch2 = gfx->RealPitch;
            gfx->PPL = gfx->PPLx2 >> 1;
        }

//	if (Settings.DisplayFrameRate)
//	    S9xDisplayFrameRate ();
//	if (gfx->InfoString)
//	    S9xDisplayString (gfx->InfoString);

	S9xDeinitUpdate (ippu->RenderedScreenWidth, ippu->RenderedScreenHeight);
    }

#ifdef CHEATS
    S9xApplyCheats ();
#endif

#ifdef DEBUGGER
    if (CPU.Flags & FRAME_ADVANCE_FLAG)
    {
	if (ICPU.FrameAdvanceCount)
	{
	    ICPU.FrameAdvanceCount--;
	    ippu->RenderThisFrame = TRUE;
	    ippu->FrameSkip = 0;
	}
	else
	{
	    CPU.Flags &= ~FRAME_ADVANCE_FLAG;
	    CPU.Flags |= DEBUG_MODE_FLAG;
	}
    }
#endif
    if (CPU.SRAMModified)
    {
		if (!CPU.AutoSaveTimer)
		{
			if (!(CPU.AutoSaveTimer = Settings.AutoSaveDelay * Memory.ROMFramesPerSecond))
			CPU.SRAMModified = FALSE;
		}
		else
		{
			if (!--CPU.AutoSaveTimer)
			{
				S9xAutoSaveSRAM ();
				CPU.SRAMModified = FALSE;
			}
		}
    }
}

void S9xSetInfoString (const char *string)
{
    GFX.InfoString = string;
    GFX.InfoStringTimeout = 120;
}

inline void SelectTileRenderer (bool8_32 normal)
{
	struct SGFX * gfx = &GFX;

    if (normal)
    {
	DrawTilePtr = DrawTile16;
	DrawClippedTilePtr = DrawClippedTile16;
	DrawLargePixelPtr = DrawLargePixel16;
    }
    else
    {
	if (gfx->r2131 & 0x80)
	{
	    if (gfx->r2131 & 0x40)
	    {
		if (gfx->r2130 & 2)
		{
		    DrawTilePtr = DrawTile16Sub1_2;
		    DrawClippedTilePtr = DrawClippedTile16Sub1_2;
		}
		else
		{
		    // Fixed colour substraction
		    DrawTilePtr = DrawTile16FixedSub1_2;
		    DrawClippedTilePtr = DrawClippedTile16FixedSub1_2;
		}
		DrawLargePixelPtr = DrawLargePixel16Sub1_2;
	    }
	    else
	    {
		DrawTilePtr = DrawTile16Sub;
		DrawClippedTilePtr = DrawClippedTile16Sub;
		DrawLargePixelPtr = DrawLargePixel16Sub;
	    }
	}
	else
	{
	    if (gfx->r2131 & 0x40)
	    {
		if (gfx->r2130 & 2)
		{
		    DrawTilePtr = DrawTile16Add1_2;
		    DrawClippedTilePtr = DrawClippedTile16Add1_2;
		}
		else
		{
		    // Fixed colour addition
		    DrawTilePtr = DrawTile16FixedAdd1_2;
		    DrawClippedTilePtr = DrawClippedTile16FixedAdd1_2;
		}
		DrawLargePixelPtr = DrawLargePixel16Add1_2;
	    }
	    else
	    {
		DrawTilePtr = DrawTile16Add;
		DrawClippedTilePtr = DrawClippedTile16Add;
		DrawLargePixelPtr = DrawLargePixel16Add;
	    }
	}
    }
}

void S9xSetupOBJ ()
{
    int SmallSize;
    int LargeSize;
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 

    switch (ppu->OBJSizeSelect)
    {
    case 0:
	SmallSize = 8;
	LargeSize = 16;
	break;
    case 1:
	SmallSize = 8;
	LargeSize = 32;
	break;
    case 2:
	SmallSize = 8;
	LargeSize = 64;
	break;
    case 3:
	SmallSize = 16;
	LargeSize = 32;
	break;
    case 4:
	SmallSize = 16;
	LargeSize = 64;
	break;
    case 5:
    default:
	SmallSize = 32;
	LargeSize = 64;
	break;
    }

    int C = 0;
    
    int FirstSprite = ppu->FirstSprite & 0x7f;
    int S = FirstSprite;
    do
    {
	int Size;
	if (ppu->OBJ [S].Size)
	    Size = LargeSize;
	else
	    Size = SmallSize;

	long VPos = ppu->OBJ [S].VPos;

	if (VPos >= ppu->ScreenHeight)
	    VPos -= 256;
	if (ppu->OBJ [S].HPos < 256 && ppu->OBJ [S].HPos > -Size &&
	    VPos < ppu->ScreenHeight && VPos > -Size)
	{
	    gfx->OBJList [C++] = S;
	    gfx->Sizes[S] = Size;
	    gfx->VPositions[S] = VPos;
	}
	S = (S + 1) & 0x7f;
    } while (S != FirstSprite);

    // Terminate the list
    gfx->OBJList [C] = -1;
    IPPU.OBJChanged = FALSE;
}

void DrawOBJS (bool8_32 OnMain = FALSE, uint8 D = 0)
{
    uint32 O;
    uint32 BaseTile, Tile;
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 

    CHECK_SOUND();

    BG.BitShift = 4;
    BG.TileShift = 5;
    BG.TileAddress = ppu->OBJNameBase;
    BG.StartPalette = 128;
    BG.PaletteShift = 4;
    BG.PaletteMask = 7;
    BG.Buffer = IPPU.TileCache [TILE_4BIT];
    BG.Buffered = IPPU.TileCached [TILE_4BIT];
    BG.NameSelect = ppu->OBJNameSelect;
    BG.DirectColourMode = FALSE;

    gfx->PixSize = 1;

    if (Settings.SupportHiRes)
    {
		if (ppu->BGMode == 5 || ppu->BGMode == 6)
		{
			gfx->PixSize = 2;
			if (IPPU.LatchedInterlace)
			{
#ifndef _ZAURUS
				if (Settings.SixteenBit)
#endif
				{
					DrawTilePtr = DrawTile16x2x2;
					DrawClippedTilePtr = DrawClippedTile16x2x2;
				}
#ifndef _ZAURUS
				else
				{
					DrawTilePtr = DrawTilex2x2;
					DrawClippedTilePtr = DrawClippedTilex2x2;
				}
#endif
			}
			else
			{
#ifndef _ZAURUS
			if (Settings.SixteenBit)
#endif
				{
					DrawTilePtr = DrawTile16x2;
					DrawClippedTilePtr = DrawClippedTile16x2;
				}
#ifndef _ZAURUS
				else
				{
					DrawTilePtr = DrawTilex2;
					DrawClippedTilePtr = DrawClippedTilex2;
				}
#endif
			}
	}
	else
	{
#ifndef _ZAURUS
	    if (Settings.SixteenBit)
#endif
	    {
		DrawTilePtr = DrawTile16;
		DrawClippedTilePtr = DrawClippedTile16;
	    }
#ifndef _ZAURUS
	    else
	    {
		DrawTilePtr = DrawTile;
		DrawClippedTilePtr = DrawClippedTile;
	    }
#endif
	}
    }
    gfx->Z1 = D + 2;

    int I = 0;
    for (int S = gfx->OBJList [I++]; S >= 0; S = gfx->OBJList [I++])
    {
	int VPos = gfx->VPositions [S];
	int Size = gfx->Sizes[S];
	int TileInc = 1;
	int Offset;

	if (VPos + Size <= (int) gfx->StartY || VPos > (int) gfx->EndY)
	    continue;

	if (OnMain && SUB_OR_ADD(4))
	{
	    SelectTileRenderer (!gfx->Pseudo && ppu->OBJ [S].Palette < 4);
	}

	BaseTile = (ppu->OBJ[S].Name | ppu->OBJ[S].Palette << 10);

	if (ppu->OBJ[S].HFlip)
	{
	    BaseTile += ((Size >> 3) - 1) | H_FLIP;
	    TileInc = -1;
	}
	if (ppu->OBJ[S].VFlip)
	    BaseTile |= V_FLIP;

	int clipcount = gfx->pCurrentClip->Count [4];
	if (!clipcount)
	    clipcount = 1;
	
	gfx->Z2 = (ppu->OBJ[S].Priority + 1) * 4 + D;

	for (int clip = 0; clip < clipcount; clip++)
	{
	    int Left; 
	    int Right;
	    if (!gfx->pCurrentClip->Count [4])
	    {
		Left = 0;
		Right = 256;
	    }
	    else
	    {
		Left = gfx->pCurrentClip->Left [clip][4];
		Right = gfx->pCurrentClip->Right [clip][4];
	    }

	    if (Right <= Left || ppu->OBJ[S].HPos + Size <= Left ||
		ppu->OBJ[S].HPos >= Right)
		continue;

	    for (int Y = 0; Y < Size; Y += 8)
	    {
		if (VPos + Y + 7 >= (int) gfx->StartY && VPos + Y <= (int) gfx->EndY)
		{
		    int StartLine;
		    int TileLine;
		    int LineCount;
		    int Last;
		    
		    if ((StartLine = VPos + Y) < (int) gfx->StartY)
		    {
			StartLine = gfx->StartY - StartLine;
			LineCount = 8 - StartLine;
		    }
		    else
		    {
			StartLine = 0;
			LineCount = 8;
		    }
		    if ((Last = VPos + Y + 7 - gfx->EndY) > 0)
			if ((LineCount -= Last) <= 0)
			    break;

		    TileLine = StartLine << 3;
		    O = (VPos + Y + StartLine) * gfx->PPL;
		    if (!ppu->OBJ[S].VFlip)
			Tile = BaseTile + (Y << 1);
		    else
			Tile = BaseTile + ((Size - Y - 8) << 1);

		    int Middle = Size >> 3;
		    if (ppu->OBJ[S].HPos < Left)
		    {
			Tile += ((Left - ppu->OBJ[S].HPos) >> 3) * TileInc;
			Middle -= (Left - ppu->OBJ[S].HPos) >> 3;
			O += Left * gfx->PixSize;
			if ((Offset = (Left - ppu->OBJ[S].HPos) & 7))
			{
			    O -= Offset * gfx->PixSize;
			    int W = 8 - Offset;
			    int Width = Right - Left;
			    if (W > Width)
				W = Width;
			    (*DrawClippedTilePtr) (Tile, O, Offset, W,
						   TileLine, LineCount, gfx);
			    
			    if (W >= Width)
				continue;
			    Tile += TileInc;
			    Middle--;
			    O += 8 * gfx->PixSize;
			}
		    }
		    else
			O += ppu->OBJ[S].HPos * gfx->PixSize;

		    if (ppu->OBJ[S].HPos + Size >= Right)
		    {
			Middle -= ((ppu->OBJ[S].HPos + Size + 7) -
				   Right) >> 3;
			Offset = (Right - (ppu->OBJ[S].HPos + Size)) & 7;
		    }
		    else
			Offset = 0;

		    for (int X = 0; X < Middle; X++, O += 8 * gfx->PixSize,
			 Tile += TileInc)
		    {
			(*DrawTilePtr) (Tile, O, TileLine, LineCount, gfx);
		    }
		    if (Offset)
		    {
			(*DrawClippedTilePtr) (Tile, O, 0, Offset,
					       TileLine, LineCount, gfx);
		    }
		}
	    }
	}
    }
}

void DrawBackgroundMosaic (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2)
{
    CHECK_SOUND();

    uint32 Tile;
    uint16 *SC0;
    uint16 *SC1;
    uint16 *SC2;
    uint16 *SC3;
    uint8 depths [2] = {Z1, Z2};
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 
    
    if (BGMode == 0)
	BG.StartPalette = bg << 5;
    else
	BG.StartPalette = 0;

    SC0 = (uint16 *) &Memory.VRAM[ppu->BG[bg].SCBase << 1];

    if (ppu->BG[bg].SCSize & 1)
	SC1 = SC0 + 1024;
    else
	SC1 = SC0;

    if (ppu->BG[bg].SCSize & 2)
	SC2 = SC1 + 1024;
    else
	SC2 = SC0;

    if (ppu->BG[bg].SCSize & 1)
	SC3 = SC2 + 1024;
    else
	SC3 = SC2;

    uint32 Lines;
    uint32 OffsetMask;
    uint32 OffsetShift;

    if (BG.TileSize == 16)
    {
	OffsetMask = 0x3ff;
	OffsetShift = 4;
    }
    else
    {
	OffsetMask = 0x1ff;
	OffsetShift = 3;
    }

    for (uint32 Y = gfx->StartY; Y <= gfx->EndY; Y += Lines)
    {
	uint32 VOffset = LineData [Y].BG[bg].VOffset;
	uint32 HOffset = LineData [Y].BG[bg].HOffset;
	uint32 MosaicOffset = Y % ppu->Mosaic;

	for (Lines = 1; Lines < ppu->Mosaic - MosaicOffset; Lines++)
	    if ((VOffset != LineData [Y + Lines].BG[bg].VOffset) ||
		(HOffset != LineData [Y + Lines].BG[bg].HOffset))
		break;
	
	uint32 MosaicLine = VOffset + Y - MosaicOffset;

	if (Y + Lines > gfx->EndY)
	    Lines = gfx->EndY + 1 - Y;
	uint32 VirtAlign = (MosaicLine & 7) << 3;
	
	uint16 *b1;
	uint16 *b2;

	uint32 ScreenLine = MosaicLine >> OffsetShift;
	uint32 Rem16 = MosaicLine & 15;

	if (ScreenLine & 0x20)
	    b1 = SC2, b2 = SC3;
	else
	    b1 = SC0, b2 = SC1;

	b1 += (ScreenLine & 0x1f) << 5;
	b2 += (ScreenLine & 0x1f) << 5;
	uint16 *t;
	uint32 Left = 0;
	uint32 Right = 256;

	uint32 ClipCount = gfx->pCurrentClip->Count [bg];
	uint32 HPos = HOffset;
	uint32 PixWidth = ppu->Mosaic;

	if (!ClipCount)
	    ClipCount = 1;

	for (uint32 clip = 0; clip < ClipCount; clip++)
	{
	    if (gfx->pCurrentClip->Count [bg])
	    {
		Left = gfx->pCurrentClip->Left [clip][bg];
		Right = gfx->pCurrentClip->Right [clip][bg];
		uint32 r = Left % ppu->Mosaic;
		HPos = HOffset + Left;
		PixWidth = ppu->Mosaic - r;
	    }
	    uint32 s = Y * gfx->PPL + Left * gfx->PixSize;
	    for (uint32 x = Left; x < Right; x += PixWidth, 
		 s += PixWidth * gfx->PixSize,
		 HPos += PixWidth, PixWidth = ppu->Mosaic)
	    {
		uint32 Quot = (HPos & OffsetMask) >> 3;

		if (x + PixWidth >= Right)
		    PixWidth = Right - x;

		if (BG.TileSize == 8)
		{
		    if (Quot > 31)
			t = b2 + (Quot & 0x1f);
		    else
			t = b1 + Quot;
		}
		else
		{
		    if (Quot > 63)
			t = b2 + ((Quot >> 1) & 0x1f);
		    else
			t = b1 + (Quot >> 1);
		}

		Tile = READ_2BYTES (t);
		gfx->Z1 = gfx->Z2 = depths [(Tile & 0x2000) >> 13];

		// Draw tile...
		if (BG.TileSize != 8)
		{
		    if (Tile & H_FLIP)
		    {
			// Horizontal flip, but what about vertical flip ?
			if (Tile & V_FLIP)
			{
			    // Both horzontal & vertical flip
			    if (Rem16 < 8)
			    {
				(*DrawLargePixelPtr) (Tile + 17 - (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines, gfx);
			    }
			    else
			    {
				(*DrawLargePixelPtr) (Tile + 1 - (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines, gfx);
			    }
			}
			else
			{
			    // Horizontal flip only
			    if (Rem16 > 7)
			    {
				(*DrawLargePixelPtr) (Tile + 17 - (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines, gfx);
			    }
			    else
			    {
				(*DrawLargePixelPtr) (Tile + 1 - (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines, gfx);
			    }
			}
		    }
		    else
		    {
			// No horizontal flip, but is there a vertical flip ?
			if (Tile & V_FLIP)
			{
			    // Vertical flip only
			    if (Rem16 < 8)
			    {
				(*DrawLargePixelPtr) (Tile + 16 + (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines, gfx);
			    }
			    else
			    {
				(*DrawLargePixelPtr) (Tile + (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines, gfx);
			    }
			}
			else
			{
			    // Normal unflipped
			    if (Rem16 > 7)
			    {
				(*DrawLargePixelPtr) (Tile + 16 + (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines, gfx);
			    }
			    else
			    {
				(*DrawLargePixelPtr) (Tile + (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines, &GFX);
			    }
			}
		    }
		}
		else
		    (*DrawLargePixelPtr) (Tile, s, HPos & 7, PixWidth,
					  VirtAlign, Lines, gfx);
	    }
	}
    }
}

void DrawBackgroundOffset (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2)
{
    CHECK_SOUND();

    uint32 Tile;
    uint16 *SC0;
    uint16 *SC1;
    uint16 *SC2;
    uint16 *SC3;
    uint16 *BPS0;
    uint16 *BPS1;
    uint16 *BPS2;
    uint16 *BPS3;
    uint32 Width;
    int VOffsetOffset = BGMode == 4 ? 0 : 32;
    uint8 depths [2] = {Z1, Z2};
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 
    
    BG.StartPalette = 0;

    BPS0 = (uint16 *) &Memory.VRAM[ppu->BG[2].SCBase << 1];

    if (ppu->BG[2].SCSize & 1)
	BPS1 = BPS0 + 1024;
    else
	BPS1 = BPS0;

    if (ppu->BG[2].SCSize & 2)
	BPS2 = BPS1 + 1024;
    else
	BPS2 = BPS0;

    if (ppu->BG[2].SCSize & 1)
	BPS3 = BPS2 + 1024;
    else
	BPS3 = BPS2;
    
    SC0 = (uint16 *) &Memory.VRAM[ppu->BG[bg].SCBase << 1];

    if (ppu->BG[bg].SCSize & 1)
	SC1 = SC0 + 1024;
    else
	SC1 = SC0;

    if (ppu->BG[bg].SCSize & 2)
	SC2 = SC1 + 1024;
    else
	SC2 = SC0;
    if (ppu->BG[bg].SCSize & 1)
	SC3 = SC2 + 1024;
    else
	SC3 = SC2;

    static const int Lines = 1;
    int OffsetMask;
    int OffsetShift;
    int OffsetEnableMask = 1 << (bg + 13);

    if (BG.TileSize == 16)
    {
	OffsetMask = 0x3ff;
	OffsetShift = 4;
    }
    else
    {
	OffsetMask = 0x1ff;
	OffsetShift = 3;
    }

    for (uint32 Y = gfx->StartY; Y <= gfx->EndY; Y++)
    {
	uint32 VOff = LineData [Y].BG[2].VOffset;
	uint32 HOff = LineData [Y].BG[2].HOffset;
	int VirtAlign;
	int ScreenLine = VOff >> 3;
	int t1;
	int t2;
	uint16 *s0;
	uint16 *s1;
	uint16 *s2;

	if (ScreenLine & 0x20)
	    s1 = BPS2, s2 = BPS3;
	else
	    s1 = BPS0, s2 = BPS1;

	s1 += (ScreenLine & 0x1f) << 5;
	s2 += (ScreenLine & 0x1f) << 5;

	int clipcount = gfx->pCurrentClip->Count [bg];
	if (!clipcount)
	    clipcount = 1;

	for (int clip = 0; clip < clipcount; clip++)
	{
	    uint32 Left;
	    uint32 Right;

	    if (!gfx->pCurrentClip->Count [bg])
	    {
		Left = 0;
		Right = 256;
	    }
	    else
	    {
		Left = gfx->pCurrentClip->Left [clip][bg];
		Right = gfx->pCurrentClip->Right [clip][bg];

		if (Right <= Left)
		    continue;
	    }

	    uint32 VOffset;
	    uint32 HOffset;
	    uint32 Offset;
	    uint32 HPos;
	    uint32 Quot;
	    uint32 Count;
	    uint16 *t;
	    uint32 Quot2;
	    uint32 VCellOffset;
	    uint32 HCellOffset;
	    uint16 *b1;
	    uint16 *b2;
	    uint32 TotalCount = 0;
	    uint32 MaxCount = 8;

	    uint32 s = Left * gfx->PixSize + Y * gfx->PPL;
	    bool8_32 left_hand_edge = (Left == 0);
	    Width = Right - Left;

	    if (Left & 7)
		MaxCount = 8 - (Left & 7);

	    while (Left < Right) 
	    {
		if (left_hand_edge)
		{
		    // The SNES offset-per-tile background mode has a
		    // hardware limitation that the offsets cannot be set
		    // for the tile at the left-hand edge of the screen.
		    VOffset = LineData [Y].BG[bg].VOffset;
		    HOffset = LineData [Y].BG[bg].HOffset;
		    left_hand_edge = FALSE;
		}
		else
		{
		    // All subsequent offset tile data is shifted left by one,
		    // hence the - 1 below.
		    Quot2 = ((HOff + Left - 1) & OffsetMask) >> 3;

		    if (Quot2 > 31)
			s0 = s2 + (Quot2 & 0x1f);
		    else
			s0 = s1 + Quot2;

		    HCellOffset = READ_2BYTES (s0);

		    if (BGMode == 4)
		    {
			VOffset = LineData [Y].BG[bg].VOffset;
			HOffset = LineData [Y].BG[bg].HOffset;
			if ((HCellOffset & OffsetEnableMask))
			{
			    if (HCellOffset & 0x8000)
				VOffset = HCellOffset + 1;
			    else
				HOffset = HCellOffset;
			}
		    }
		    else
		    {
			VCellOffset = READ_2BYTES (s0 + VOffsetOffset);
			if ((VCellOffset & OffsetEnableMask))
			    VOffset = VCellOffset + 1;
			else
			    VOffset = LineData [Y].BG[bg].VOffset;

			if ((HCellOffset & OffsetEnableMask))
			    HOffset = HCellOffset;
			else
			    HOffset = LineData [Y].BG[bg].HOffset - 
				      Settings.StrikeGunnerOffsetHack;
		    }
		}
		VirtAlign = ((Y + VOffset) & 7) << 3;
		ScreenLine = (VOffset + Y) >> OffsetShift;

		if (((VOffset + Y) & 15) > 7)
		{
		    t1 = 16;
		    t2 = 0;
		}
		else
		{
		    t1 = 0;
		    t2 = 16;
		}

		if (ScreenLine & 0x20)
		    b1 = SC2, b2 = SC3;
		else
		    b1 = SC0, b2 = SC1;

		b1 += (ScreenLine & 0x1f) << 5;
		b2 += (ScreenLine & 0x1f) << 5;

		HPos = (HOffset + Left) & OffsetMask;

		Quot = HPos >> 3;

		if (BG.TileSize == 8)
		{
		    if (Quot > 31)
			t = b2 + (Quot & 0x1f);
		    else
			t = b1 + Quot;
		}
		else
		{
		    if (Quot > 63)
			t = b2 + ((Quot >> 1) & 0x1f);
		    else
			t = b1 + (Quot >> 1);
		}

		if (MaxCount + TotalCount > Width)
		    MaxCount = Width - TotalCount;

		Offset = HPos & 7;

		Count = 8 - Offset;
		if (Count > MaxCount)
		    Count = MaxCount;

		s -= Offset * gfx->PixSize;
		Tile = READ_2BYTES(t);
		gfx->Z1 = gfx->Z2 = depths [(Tile & 0x2000) >> 13];

		if (BG.TileSize == 8)
		    (*DrawClippedTilePtr) (Tile, s, Offset, Count, VirtAlign, Lines, gfx);
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
			(*DrawClippedTilePtr) (Tile + t1 + (Quot & 1),
					       s, Offset, Count, VirtAlign, Lines, gfx);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawClippedTilePtr) (Tile + t2 + 1 - (Quot & 1),
						   s, Offset, Count, VirtAlign, Lines, gfx);
			}
			else
			{
			    // H flip only
			    (*DrawClippedTilePtr) (Tile + t1 + 1 - (Quot & 1),
						   s, Offset, Count, VirtAlign, Lines, gfx);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawClippedTilePtr) (Tile + t2 + (Quot & 1),
					       s, Offset, Count, VirtAlign, Lines, gfx);
		    }
		}

		Left += Count;
		TotalCount += Count;
		s += (Offset + Count) * gfx->PixSize;
		MaxCount = 8;
	    }
	}
    }
}

void DrawBackgroundMode5 (uint32 /* BGMODE */, uint32 bg, uint8 Z1, uint8 Z2)
{
    CHECK_SOUND();
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 

    gfx->Pitch = gfx->RealPitch;
    gfx->PPL = gfx->PPLx2 >> 1;
    gfx->PixSize = 1;
    uint8 depths [2] = {Z1, Z2};

    uint32 Tile;
    uint16 *SC0;
    uint16 *SC1;
    uint16 *SC2;
    uint16 *SC3;
    uint32 Width;
    
    BG.StartPalette = 0;

    SC0 = (uint16 *) &Memory.VRAM[ppu->BG[bg].SCBase << 1];

    if ((ppu->BG[bg].SCSize & 1))
	SC1 = SC0 + 1024;
    else
	SC1 = SC0;

    if ((ppu->BG[bg].SCSize & 2))
	SC2 = SC1 + 1024;
    else
	SC2 = SC0;

    if ((ppu->BG[bg].SCSize & 1))
	SC3 = SC2 + 1024;
    else
	SC3 = SC2;
    
    int Lines;
    int VOffsetMask;
    int VOffsetShift;

    if (BG.TileSize == 16)
    {
	VOffsetMask = 0x3ff;
	VOffsetShift = 4;
    }
    else
    {
	VOffsetMask = 0x1ff;
	VOffsetShift = 3;
    }
    int endy = IPPU.LatchedInterlace ? gfx->EndY << 1 : gfx->EndY;

    for (int Y = IPPU.LatchedInterlace ? gfx->StartY << 1 : gfx->StartY; Y <= endy; Y += Lines)
    {
	int y = IPPU.LatchedInterlace ? (Y >> 1) : Y;
	uint32 VOffset = LineData [y].BG[bg].VOffset;
	uint32 HOffset = LineData [y].BG[bg].HOffset;
	int VirtAlign = (Y + VOffset) & 7;
	
	for (Lines = 1; Lines < 8 - VirtAlign; Lines++)
	    if ((VOffset != LineData [y + Lines].BG[bg].VOffset) ||
		(HOffset != LineData [y + Lines].BG[bg].HOffset))
		break;

	HOffset <<= 1;
	if (Y + Lines > endy)
	    Lines = endy + 1 - Y;
	VirtAlign <<= 3;
	
	int ScreenLine = (VOffset + Y) >> VOffsetShift;
	int t1;
	int t2;
	if (((VOffset + Y) & 15) > 7)
	{
	    t1 = 16;
	    t2 = 0;
	}
	else
	{
	    t1 = 0;
	    t2 = 16;
	}
	uint16 *b1;
	uint16 *b2;

	if (ScreenLine & 0x20)
	    b1 = SC2, b2 = SC3;
	else
	    b1 = SC0, b2 = SC1;

	b1 += (ScreenLine & 0x1f) << 5;
	b2 += (ScreenLine & 0x1f) << 5;

	int clipcount = gfx->pCurrentClip->Count [bg];
	if (!clipcount)
	    clipcount = 1;
	for (int clip = 0; clip < clipcount; clip++)
	{
	    int Left;
	    int Right;

	    if (!gfx->pCurrentClip->Count [bg])
	    {
		Left = 0;
		Right = 512;
	    }
	    else
	    {
		Left = gfx->pCurrentClip->Left [clip][bg] * 2;
		Right = gfx->pCurrentClip->Right [clip][bg] * 2;

		if (Right <= Left)
		    continue;
	    }

	    uint32 s = Left * gfx->PixSize + Y * gfx->PPL;
	    uint32 HPos = (HOffset + Left * gfx->PixSize) & 0x3ff;

	    uint32 Quot = HPos >> 3;
	    uint32 Count = 0;
	    
	    uint16 *t;
	    if (Quot > 63)
		t = b2 + ((Quot >> 1) & 0x1f);
	    else
		t = b1 + (Quot >> 1);

	    Width = Right - Left;
	    // Left hand edge clipped tile
	    if (HPos & 7)
	    {
		int Offset = (HPos & 7);
		Count = 8 - Offset;
		if (Count > Width)
		    Count = Width;
		s -= Offset;
		Tile = READ_2BYTES (t);
		gfx->Z1 = gfx->Z2 = depths [(Tile & 0x2000) >> 13];

		if (BG.TileSize == 8)
		{
		    if (!(Tile & H_FLIP))
		    {
			// Normal, unflipped
			(*DrawHiResClippedTilePtr) (Tile + (Quot & 1),
						    s, Offset, Count, VirtAlign, Lines, gfx);
		    }
		    else
		    {
			// H flip
			(*DrawHiResClippedTilePtr) (Tile + 1 - (Quot & 1),
						    s, Offset, Count, VirtAlign, Lines, gfx);
		    }
		}
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
			(*DrawHiResClippedTilePtr) (Tile + t1 + (Quot & 1),
						    s, Offset, Count, VirtAlign, Lines, gfx);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawHiResClippedTilePtr) (Tile + t2 + 1 - (Quot & 1),
							s, Offset, Count, VirtAlign, Lines, gfx);
			}
			else
			{
			    // H flip only
			    (*DrawHiResClippedTilePtr) (Tile + t1 + 1 - (Quot & 1),
							s, Offset, Count, VirtAlign, Lines, gfx);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawHiResClippedTilePtr) (Tile + t2 + (Quot & 1),
						    s, Offset, Count, VirtAlign, Lines, gfx);
		    }
		}

		t += Quot & 1;
		if (Quot == 63)
		    t = b2;
		else if (Quot == 127)
		    t = b1;
		Quot++;
		s += 8;
	    }

	    // Middle, unclipped tiles
	    Count = Width - Count;
	    int Middle = Count >> 3;
	    Count &= 7;
	    for (int C = Middle; C > 0; s += 8, Quot++, C--)
	    {
		Tile = READ_2BYTES(t);
		gfx->Z1 = gfx->Z2 = depths [(Tile & 0x2000) >> 13];
		if (BG.TileSize == 8)
		{
		    if (!(Tile & H_FLIP))
		    {
			// Normal, unflipped
			(*DrawHiResTilePtr) (Tile + (Quot & 1),
					     s, VirtAlign, Lines, gfx);
		    }
		    else
		    {
			// H flip
			(*DrawHiResTilePtr) (Tile + 1 - (Quot & 1),
					    s, VirtAlign, Lines, gfx);
		    }
		}
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
			(*DrawHiResTilePtr) (Tile + t1 + (Quot & 1),
					     s, VirtAlign, Lines, gfx);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawHiResTilePtr) (Tile + t2 + 1 - (Quot & 1),
						 s, VirtAlign, Lines, gfx);
			}
			else
			{
			    // H flip only
			    (*DrawHiResTilePtr) (Tile + t1 + 1 - (Quot & 1),
						 s, VirtAlign, Lines, gfx);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawHiResTilePtr) (Tile + t2 + (Quot & 1),
					     s, VirtAlign, Lines, gfx);
		    }
		}

		t += Quot & 1;
		if (Quot == 63)
		    t = b2;
		else
		if (Quot == 127)
		    t = b1;
	    }

	    // Right-hand edge clipped tiles
	    if (Count)
	    {
		Tile = READ_2BYTES(t);
		gfx->Z1 = gfx->Z2 = depths [(Tile & 0x2000) >> 13];
		if (BG.TileSize == 8)
		{
		    if (!(Tile & H_FLIP))
		    {
			// Normal, unflipped
			(*DrawHiResClippedTilePtr) (Tile + (Quot & 1),
						    s, 0, Count, VirtAlign, Lines, gfx);
		    }
		    else
		    {
			// H flip
			(*DrawHiResClippedTilePtr) (Tile + 1 - (Quot & 1),
						    s, 0, Count, VirtAlign, Lines, gfx);
		    }
		}
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
			(*DrawHiResClippedTilePtr) (Tile + t1 + (Quot & 1),
						    s, 0, Count, VirtAlign, Lines, gfx);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawHiResClippedTilePtr) (Tile + t2 + 1 - (Quot & 1),
							s, 0, Count, VirtAlign, Lines, gfx);
			}
			else
			{
			    // H flip only
			    (*DrawHiResClippedTilePtr) (Tile + t1 + 1 - (Quot & 1),
							s, 0, Count, VirtAlign, Lines, gfx);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawHiResClippedTilePtr) (Tile + t2 + (Quot & 1),
						    s, 0, Count, VirtAlign, Lines, gfx);
		    }
		}
	    }
	}
    }
    gfx->Pitch = IPPU.LatchedInterlace ? gfx->RealPitch * 2 : gfx->RealPitch;
    gfx->PPL = IPPU.LatchedInterlace ? gfx->PPLx2 : (gfx->PPLx2 >> 1);
}

void DrawBackground (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2)
{
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 
    gfx->PixSize = 1;

    BG.TileSize = BGSizes [ppu->BG[bg].BGSize];
    BG.BitShift = BitShifts[BGMode][bg];
    BG.TileShift = TileShifts[BGMode][bg];
    BG.TileAddress = ppu->BG[bg].NameBase << 1;
    BG.NameSelect = 0;
    BG.Buffer = IPPU.TileCache [Depths [BGMode][bg]];
    BG.Buffered = IPPU.TileCached [Depths [BGMode][bg]];
    BG.PaletteShift = PaletteShifts[BGMode][bg];
    BG.PaletteMask = PaletteMasks[BGMode][bg];
    BG.DirectColourMode = (BGMode == 3 || BGMode == 4) && bg == 0 &&
		          (gfx->r2130 & 1);

    if (ppu->BGMosaic [bg] && ppu->Mosaic > 1)
    {
	DrawBackgroundMosaic (BGMode, bg, Z1, Z2);
	return;

    }
    switch (BGMode)
    {
    case 2:
	if (Settings.WrestlemaniaArcade)
	    break;
    case 4: // Used by Puzzle Bobble
        DrawBackgroundOffset (BGMode, bg, Z1, Z2);
	return;

    case 5:
    case 6: // XXX: is also offset per tile.
		if (Settings.SupportHiRes)
	{
	    DrawBackgroundMode5 (BGMode, bg, Z1, Z2);
	    return;
	}
	break;
    }
    CHECK_SOUND();

    uint32 Tile;
    uint16 *SC0;
    uint16 *SC1;
    uint16 *SC2;
    uint16 *SC3;
    uint32 Width;
    uint8 depths [2] = {Z1, Z2};
    
    if (BGMode == 0)
	BG.StartPalette = bg << 5;
    else
	BG.StartPalette = 0;

    SC0 = (uint16 *) &Memory.VRAM[ppu->BG[bg].SCBase << 1];

    if (ppu->BG[bg].SCSize & 1)
	SC1 = SC0 + 1024;
    else
	SC1 = SC0;

    if (ppu->BG[bg].SCSize & 2)
	SC2 = SC1 + 1024;
    else
	SC2 = SC0;

    if (ppu->BG[bg].SCSize & 1)
	SC3 = SC2 + 1024;
    else
	SC3 = SC2;
    
    int Lines;
    int OffsetMask;
    int OffsetShift;

    if (BG.TileSize == 16)
    {
	OffsetMask = 0x3ff;
	OffsetShift = 4;
    }
    else
    {
	OffsetMask = 0x1ff;
	OffsetShift = 3;
    }

    for (uint32 Y = gfx->StartY; Y <= gfx->EndY; Y += Lines)
    {
	uint32 VOffset = LineData [Y].BG[bg].VOffset;
	uint32 HOffset = LineData [Y].BG[bg].HOffset;
	int VirtAlign = (Y + VOffset) & 7;
	
	for (Lines = 1; Lines < 8 - VirtAlign; Lines++)
	    if ((VOffset != LineData [Y + Lines].BG[bg].VOffset) ||
		(HOffset != LineData [Y + Lines].BG[bg].HOffset))
		break;

	if (Y + Lines > gfx->EndY)
	    Lines = gfx->EndY + 1 - Y;

	VirtAlign <<= 3;
	
	uint32 ScreenLine = (VOffset + Y) >> OffsetShift;
	uint32 t1;
	uint32 t2;
	if (((VOffset + Y) & 15) > 7)
	{
	    t1 = 16;
	    t2 = 0;
	}
	else
	{
	    t1 = 0;
	    t2 = 16;
	}
	uint16 *b1;
	uint16 *b2;

	if (ScreenLine & 0x20)
	    b1 = SC2, b2 = SC3;
	else
	    b1 = SC0, b2 = SC1;

	b1 += (ScreenLine & 0x1f) << 5;
	b2 += (ScreenLine & 0x1f) << 5;

	int clipcount = gfx->pCurrentClip->Count [bg];
	if (!clipcount)
	    clipcount = 1;
	for (int clip = 0; clip < clipcount; clip++)
	{
	    uint32 Left;
	    uint32 Right;

	    if (!gfx->pCurrentClip->Count [bg])
	    {
		Left = 0;
		Right = 256;
	    }
	    else
	    {
		Left = gfx->pCurrentClip->Left [clip][bg];
		Right = gfx->pCurrentClip->Right [clip][bg];

		if (Right <= Left)
		    continue;
	    }

	    uint32 s = Left * gfx->PixSize + Y * gfx->PPL;
	    uint32 HPos = (HOffset + Left) & OffsetMask;

	    uint32 Quot = HPos >> 3;
	    uint32 Count = 0;
	    
	    uint16 *t;
	    if (BG.TileSize == 8)
	    {
		if (Quot > 31)
		    t = b2 + (Quot & 0x1f);
		else
		    t = b1 + Quot;
	    }
	    else
	    {
		if (Quot > 63)
		    t = b2 + ((Quot >> 1) & 0x1f);
		else
		    t = b1 + (Quot >> 1);
	    }

	    Width = Right - Left;
	    // Left hand edge clipped tile
	    if (HPos & 7)
	    {
		uint32 Offset = (HPos & 7);
		Count = 8 - Offset;
		if (Count > Width)
		    Count = Width;
		s -= Offset * gfx->PixSize;
		Tile = READ_2BYTES(t);
		gfx->Z1 = gfx->Z2 = depths [(Tile & 0x2000) >> 13];

		if (BG.TileSize == 8)
		{
		    (*DrawClippedTilePtr) (Tile, s, Offset, Count, VirtAlign,
					   Lines, gfx);
		}
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
				// Normal, unflipped
				(*DrawClippedTilePtr) (Tile + t1 + (Quot & 1),
					       s, Offset, Count, VirtAlign, Lines, gfx);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawClippedTilePtr) (Tile + t2 + 1 - (Quot & 1),
						   s, Offset, Count, VirtAlign, Lines, gfx);
			}
			else
			{
			    // H flip only
			    (*DrawClippedTilePtr) (Tile + t1 + 1 - (Quot & 1),
						   s, Offset, Count, VirtAlign, Lines, gfx);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawClippedTilePtr) (Tile + t2 + (Quot & 1), s, 
					       Offset, Count, VirtAlign, Lines, gfx);
		    }
		}

		if (BG.TileSize == 8)
		{
		    t++;
		    if (Quot == 31)
			t = b2;
		    else if (Quot == 63)
			t = b1;
		}
		else
		{
		    t += Quot & 1;
		    if (Quot == 63)
			t = b2;
		    else if (Quot == 127)
			t = b1;
		}
		Quot++;
		s += 8 * gfx->PixSize;
	    }

	    // Middle, unclipped tiles
	    Count = Width - Count;
	    int Middle = Count >> 3;
	    Count &= 7;
	    for (int C = Middle; C > 0; s += 8 * gfx->PixSize, Quot++, C--)
	    {
		Tile = READ_2BYTES(t);
		gfx->Z1 = gfx->Z2 = depths [(Tile & 0x2000) >> 13];

		if (BG.TileSize != 8)
		{
		    if (Tile & H_FLIP)
		    {
			// Horizontal flip, but what about vertical flip ?
			if (Tile & V_FLIP)
			{
			    // Both horzontal & vertical flip
			    (*DrawTilePtr) (Tile + t2 + 1 - (Quot & 1), s, 
					    VirtAlign, Lines, gfx);
			}
			else
			{
			    // Horizontal flip only
			    (*DrawTilePtr) (Tile + t1 + 1 - (Quot & 1), s, 
					    VirtAlign, Lines, gfx);
			}
		    }
		    else
		    {
			// No horizontal flip, but is there a vertical flip ?
			if (Tile & V_FLIP)
			{
			    // Vertical flip only
			    (*DrawTilePtr) (Tile + t2 + (Quot & 1), s,
					    VirtAlign, Lines, gfx);
			}
			else
			{
			    // Normal unflipped
			    (*DrawTilePtr) (Tile + t1 + (Quot & 1), s,
					    VirtAlign, Lines, gfx);
			}
		    }
		}
		else
		{
		    (*DrawTilePtr) (Tile, s, VirtAlign, Lines, gfx);
		}

		if (BG.TileSize == 8)
		{
		    t++;
		    if (Quot == 31)
			t = b2;
		    else
		    if (Quot == 63)
			t = b1;
		}
		else
		{
		    t += Quot & 1;
		    if (Quot == 63)
			t = b2;
		    else
		    if (Quot == 127)
			t = b1;
		}
	    }
	    // Right-hand edge clipped tiles
	    if (Count)
	    {
		Tile = READ_2BYTES(t);
		gfx->Z1 = gfx->Z2 = depths [(Tile & 0x2000) >> 13];

		if (BG.TileSize == 8)
		    (*DrawClippedTilePtr) (Tile, s, 0, Count, VirtAlign, 
					   Lines, gfx);
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
			(*DrawClippedTilePtr) (Tile + t1 + (Quot & 1), s, 0, 
					       Count, VirtAlign, Lines, gfx);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawClippedTilePtr) (Tile + t2 + 1 - (Quot & 1),
						   s, 0, Count, VirtAlign, 
						   Lines, gfx);
			}
			else
			{
			    // H flip only
			    (*DrawClippedTilePtr) (Tile + t1 + 1 - (Quot & 1),
						   s, 0, Count, VirtAlign,
						   Lines, gfx);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawClippedTilePtr) (Tile + t2 + (Quot & 1),
					       s, 0, Count, VirtAlign, 
					       Lines, gfx);
		    }
		}
	    }
	}
    }
}

#define RENDER_BACKGROUND_MODE7(TYPE,FUNC) \
    CHECK_SOUND(); \
\
    uint8 *VRAM1 = Memory.VRAM + 1; \
    if (gfx->r2130 & 1) \
    { \
		if (IPPU.DirectColourMapsNeedRebuild) \
			S9xBuildDirectColourMaps (); \
		gfx->ScreenColors = DirectColourMaps [0]; \
    } \
    else \
		gfx->ScreenColors = IPPU.ScreenColors; \
\
    int aa, cc; \
    int dir; \
    int startx, endx; \
    uint32 Left = 0; \
    uint32 Right = 256; \
    uint32 ClipCount = gfx->pCurrentClip->Count [bg]; \
\
    if (!ClipCount) \
	ClipCount = 1; \
\
    Screen += gfx->StartY * gfx->Pitch; \
    uint8 *Depth = gfx->DB + gfx->StartY * gfx->PPL; \
    struct SLineMatrixData *l = &LineMatrixData [gfx->StartY]; \
\
    for (uint32 Line = gfx->StartY; Line <= gfx->EndY; Line++, Screen += gfx->Pitch, Depth += gfx->PPL, l++) \
    { \
	int yy; \
\
	int32 HOffset = ((int32) LineData [Line].BG[0].HOffset << M7) >> M7; \
	int32 VOffset = ((int32) LineData [Line].BG[0].VOffset << M7) >> M7; \
\
	int32 CentreX = ((int32) l->CentreX << M7) >> M7; \
	int32 CentreY = ((int32) l->CentreY << M7) >> M7; \
\
	if (ppu->Mode7VFlip) \
	    yy = 261 - (int) Line; \
	else \
	    yy = Line; \
\
	if (ppu->Mode7Repeat == 0) \
	    yy += (VOffset - CentreY) % 1023; \
	else \
	    yy += VOffset - CentreY; \
	int BB = l->MatrixB * yy + (CentreX << 8); \
	int DD = l->MatrixD * yy + (CentreY << 8); \
\
	for (uint32 clip = 0; clip < ClipCount; clip++) \
	{ \
	    if (gfx->pCurrentClip->Count [bg]) \
	    { \
			Left = gfx->pCurrentClip->Left [clip][bg]; \
			Right = gfx->pCurrentClip->Right [clip][bg]; \
			if (Right <= Left) \
				continue; \
	    } \
	    TYPE *p = (TYPE *) Screen + Left; \
	    uint8 *d = Depth + Left; \
\
	    if (ppu->Mode7HFlip) \
	    { \
			startx = Right - 1; \
			endx = Left - 1; \
			dir = -1; \
			aa = -l->MatrixA; \
			cc = -l->MatrixC; \
	    } \
	    else \
	    { \
			startx = Left; \
			endx = Right; \
			dir = 1; \
			aa = l->MatrixA; \
			cc = l->MatrixC; \
	    } \
	    int xx; \
	    if (ppu->Mode7Repeat == 0) \
			xx = startx + (HOffset - CentreX) % 1023; \
	    else \
			xx = startx + HOffset - CentreX; \
	    int AA = l->MatrixA * xx; \
	    int CC = l->MatrixC * xx; \
\
	    if (!ppu->Mode7Repeat) \
	    { \
		for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
		{ \
		    int X = ((AA + BB) >> 8) & 0x3ff; \
		    int Y = ((CC + DD) >> 8) & 0x3ff; \
		    uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
		    uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
		    gfx->Z1 = Mode7Depths [(b & gfx->Mode7PriorityMask) >> 7]; \
		    if (gfx->Z1 > *d && b) \
		    { \
				*p = (FUNC); \
				*d = gfx->Z1; \
		    } \
		} \
	    } \
	    else \
	    { \
		for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
		{ \
		    int X = ((AA + BB) >> 8); \
		    int Y = ((CC + DD) >> 8); \
\
		    if (Settings.Dezaemon && ppu->Mode7Repeat == 2) \
		    { \
				X &= 0x7ff; \
				Y &= 0x7ff; \
		    } \
\
		    if (((X | Y) & ~0x3ff) == 0) \
		    { \
				uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
				uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				gfx->Z1 = Mode7Depths [(b & gfx->Mode7PriorityMask) >> 7]; \
				if (gfx->Z1 > *d && b) \
				{ \
					*p = (FUNC); \
					*d = gfx->Z1; \
				} \
		    } \
		    else \
		    { \
				if (ppu->Mode7Repeat == 3) \
				{ \
					X = (x + HOffset) & 7; \
					Y = (yy + CentreY) & 7; \
					uint32 b = *(VRAM1 + ((Y & 7) << 4) + ((X & 7) << 1)); \
					gfx->Z1 = Mode7Depths [(b & gfx->Mode7PriorityMask) >> 7]; \
					if (gfx->Z1 > *d && b) \
					{ \
						*p = (FUNC); \
						*d = gfx->Z1; \
					} \
				} \
		    } \
		} \
	    } \
	} \
    }
#ifndef _ZAURUS
void DrawBGMode7Background (uint8 *Screen, int bg)
{
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 

    RENDER_BACKGROUND_MODE7 (uint8, (uint8) (b & gfx->Mode7Mask))
}
#endif
void DrawBGMode7Background16 (uint8 *Screen, int bg)
{
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 

    RENDER_BACKGROUND_MODE7 (uint16, gfx->ScreenColors [b & gfx->Mode7Mask]);
}

void DrawBGMode7Background16Add (uint8 *Screen, int bg)
{
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 

    RENDER_BACKGROUND_MODE7 (uint16, *(d + gfx->DepthDelta) ?
					(*(d + gfx->DepthDelta) != 1 ?
					    COLOR_ADD (gfx->ScreenColors [b & gfx->Mode7Mask],
						       p [gfx->Delta]) :
					    COLOR_ADD (gfx->ScreenColors [b & gfx->Mode7Mask],
						       gfx->FixedColour)) :
					 gfx->ScreenColors [b & gfx->Mode7Mask]);
}

void DrawBGMode7Background16Add1_2 (uint8 *Screen, int bg)
{
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 

    RENDER_BACKGROUND_MODE7 (uint16, *(d + gfx->DepthDelta) ?
					(*(d + gfx->DepthDelta) != 1 ?
					    COLOR_ADD1_2 (gfx->ScreenColors [b & gfx->Mode7Mask],
						       p [gfx->Delta]) :
					    COLOR_ADD (gfx->ScreenColors [b & gfx->Mode7Mask],
						       gfx->FixedColour)) :
					 gfx->ScreenColors [b & gfx->Mode7Mask]);
}

void DrawBGMode7Background16Sub (uint8 *Screen, int bg)
{
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 

    RENDER_BACKGROUND_MODE7 (uint16, *(d + gfx->DepthDelta) ?
					(*(d + gfx->DepthDelta) != 1 ?
					    COLOR_SUB (gfx->ScreenColors [b & gfx->Mode7Mask],
						       p [gfx->Delta]) :
					    COLOR_SUB (gfx->ScreenColors [b & gfx->Mode7Mask],
						       gfx->FixedColour)) :
					 gfx->ScreenColors [b & gfx->Mode7Mask]);
}

void DrawBGMode7Background16Sub1_2 (uint8 *Screen, int bg)
{
	struct SPPU * ppu = &PPU;
	struct SGFX * gfx = &GFX; 

    RENDER_BACKGROUND_MODE7 (uint16, *(d + gfx->DepthDelta) ?
					(*(d + gfx->DepthDelta) != 1 ?
					    COLOR_SUB1_2 (gfx->ScreenColors [b & gfx->Mode7Mask],
						       p [gfx->Delta]) :
					    COLOR_SUB (gfx->ScreenColors [b & gfx->Mode7Mask],
						       gfx->FixedColour)) :
					 gfx->ScreenColors [b & gfx->Mode7Mask]);
}

#ifndef _ZAURUS
#define RENDER_BACKGROUND_MODE7_i(TYPE,FUNC,COLORFUNC) \
    CHECK_SOUND(); \
\
    uint8 *VRAM1 = Memory.VRAM + 1; \
    if (GFX.r2130 & 1) \
    { \
        if (IPPU.DirectColourMapsNeedRebuild) \
            S9xBuildDirectColourMaps (); \
        GFX.ScreenColors = DirectColourMaps [0]; \
    } \
    else \
        GFX.ScreenColors = IPPU.ScreenColors; \
    \
    int aa, cc; \
    int dir; \
    int startx, endx; \
    uint32 Left = 0; \
    uint32 Right = 256; \
    uint32 ClipCount = GFX.pCurrentClip->Count [bg]; \
    \
    if (!ClipCount) \
        ClipCount = 1; \
    \
    Screen += GFX.StartY * GFX.Pitch; \
    uint8 *Depth = GFX.DB + GFX.StartY * GFX.PPL; \
    struct SLineMatrixData *l = &LineMatrixData [GFX.StartY]; \
    bool8_32 allowSimpleCase = FALSE; \
    if (!l->MatrixB && !l->MatrixC && (l->MatrixA == 0x0100) && (l->MatrixD == 0x0100) \
        && !LineMatrixData[GFX.EndY].MatrixB && !LineMatrixData[GFX.EndY].MatrixC \
        && (LineMatrixData[GFX.EndY].MatrixA == 0x0100) && (LineMatrixData[GFX.EndY].MatrixD == 0x0100) \
        ) \
        allowSimpleCase = TRUE;  \
    \
    for (uint32 Line = GFX.StartY; Line <= GFX.EndY; Line++, Screen += GFX.Pitch, Depth += GFX.PPL, l++) \
    { \
        int yy; \
        \
        int HOffset = ((int) LineData [Line].BG[0].HOffset << M7) >> M7; \
        int VOffset = ((int) LineData [Line].BG[0].VOffset << M7) >> M7; \
        \
        int CentreX = ((int) l->CentreX << M7) >> M7; \
        int CentreY = ((int) l->CentreY << M7) >> M7; \
        \
        if (PPU.Mode7VFlip) \
            yy = 261 - (int) Line; \
        else \
            yy = Line; \
        \
	if (PPU.Mode7Repeat == 0) \
	    yy += (VOffset - CentreY) % 1023; \
	else \
	    yy += VOffset - CentreY; \
        bool8_32 simpleCase = FALSE; \
        int BB; \
        int DD; \
        /* Make a special case for the identity matrix, since it's a common case and */ \
        /* can be done much more quickly without special effects */ \
        if (allowSimpleCase && !l->MatrixB && !l->MatrixC && (l->MatrixA == 0x0100) && (l->MatrixD == 0x0100)) \
        { \
            BB = CentreX << 8; \
            DD = (yy + CentreY) << 8; \
            simpleCase = TRUE; \
        } \
        else \
        { \
            BB = l->MatrixB * yy + (CentreX << 8); \
            DD = l->MatrixD * yy + (CentreY << 8); \
        } \
        \
        for (uint32 clip = 0; clip < ClipCount; clip++) \
        { \
            if (GFX.pCurrentClip->Count [bg]) \
            { \
                Left = GFX.pCurrentClip->Left [clip][bg]; \
                Right = GFX.pCurrentClip->Right [clip][bg]; \
                if (Right <= Left) \
                    continue; \
            } \
            TYPE *p = (TYPE *) Screen + Left; \
            uint8 *d = Depth + Left; \
            \
            if (PPU.Mode7HFlip) \
            { \
                startx = Right - 1; \
                endx = Left - 1; \
                dir = -1; \
                aa = -l->MatrixA; \
                cc = -l->MatrixC; \
            } \
            else \
            { \
                startx = Left; \
                endx = Right; \
                dir = 1; \
                aa = l->MatrixA; \
                cc = l->MatrixC; \
            } \
            int xx; \
	    if (PPU.Mode7Repeat == 0) \
		xx = startx + (HOffset - CentreX) % 1023; \
	    else \
		xx = startx + HOffset - CentreX; \
            int AA, CC = 0; \
            if (simpleCase) \
            { \
                AA = xx << 8; \
            } \
            else \
            { \
                AA = l->MatrixA * xx; \
                CC = l->MatrixC * xx; \
            } \
            if (simpleCase) \
            { \
                if (!PPU.Mode7Repeat) \
                { \
                    int x = startx; \
                    do \
                    { \
                        int X = ((AA + BB) >> 8) & 0x3ff; \
                        int Y = (DD >> 8) & 0x3ff; \
                        uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
                        uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
			GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                        if (GFX.Z1 > *d && b) \
                        { \
                            TYPE theColor = COLORFUNC; \
                            *p = (FUNC) | ALPHA_BITS_MASK; \
                            *d = GFX.Z1; \
                        } \
                        AA += aa, p++, d++; \
			x += dir; \
                    } while (x != endx); \
                } \
                else \
                { \
                    int x = startx; \
                    do { \
                        int X = (AA + BB) >> 8; \
                        int Y = DD >> 8; \
\
			if(Settings.Dezaemon && PPU.Mode7Repeat == 2) \
			{ \
			    X &= 0x7ff; \
			    Y &= 0x7ff; \
			} \
\
                        if (((X | Y) & ~0x3ff) == 0) \
                        { \
                            uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
			    uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
			    GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                            if (GFX.Z1 > *d && b) \
                            { \
                                TYPE theColor = COLORFUNC; \
                                *p = (FUNC) | ALPHA_BITS_MASK; \
                                *d = GFX.Z1; \
                            } \
                        } \
                        else if (PPU.Mode7Repeat == 3) \
                        { \
                            X = (x + HOffset) & 7; \
                            Y = (yy + CentreY) & 7; \
			    uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
			    uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
			    GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                            if (GFX.Z1 > *d && b) \
                            { \
                                TYPE theColor = COLORFUNC; \
                                *p = (FUNC) | ALPHA_BITS_MASK; \
                                *d = GFX.Z1; \
                            } \
                        } \
                        AA += aa; p++; d++; \
			x += dir; \
                    } while (x != endx); \
                } \
            } \
            else if (!PPU.Mode7Repeat) \
            { \
                /* The bilinear interpolator: get the colors at the four points surrounding */ \
                /* the location of one point in the _sampled_ image, and weight them according */ \
                /* to their (city block) distance.  It's very smooth, but blurry with "close up" */ \
                /* points. */ \
                \
                /* 460 (slightly less than 2 source pixels per displayed pixel) is an educated */ \
                /* guess for where bilinear filtering will become a poor method for averaging. */ \
                /* (When reducing the image, the weighting used by a bilinear filter becomes */ \
                /* arbitrary, and a simple mean is a better way to represent the source image.) */ \
                /* You can think of this as a kind of mipmapping. */ \
                if ((aa < 460 && aa > -460) && (cc < 460 && cc > -460)) \
                {\
                    for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
                    { \
                        uint32 xPos = AA + BB; \
                        uint32 xPix = xPos >> 8; \
                        uint32 yPos = CC + DD; \
                        uint32 yPix = yPos >> 8; \
                        uint32 X = xPix & 0x3ff; \
                        uint32 Y = yPix & 0x3ff; \
                        uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
			uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
			GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                        if (GFX.Z1 > *d && b) \
                        { \
                            /* X10 and Y01 are the X and Y coordinates of the next source point over. */ \
                            uint32 X10 = (xPix + dir) & 0x3ff; \
                            uint32 Y01 = (yPix + dir) & 0x3ff; \
                            uint8 *TileData10 = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X10 >> 2) & ~1)] << 7); \
                            uint8 *TileData11 = VRAM1 + (Memory.VRAM[((Y01 & ~7) << 5) + ((X10 >> 2) & ~1)] << 7); \
                            uint8 *TileData01 = VRAM1 + (Memory.VRAM[((Y01 & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
                            uint32 p1 = COLORFUNC; \
                            p1 = (p1 & FIRST_THIRD_COLOR_MASK) | ((p1 & SECOND_COLOR_MASK) << 16); \
                            b = *(TileData10 + ((Y & 7) << 4) + ((X10 & 7) << 1)); \
                            uint32 p2 = COLORFUNC; \
                            p2 = (p2 & FIRST_THIRD_COLOR_MASK) | ((p2 & SECOND_COLOR_MASK) << 16); \
                            b = *(TileData11 + ((Y01 & 7) << 4) + ((X10 & 7) << 1)); \
                            uint32 p4 = COLORFUNC; \
                            p4 = (p4 & FIRST_THIRD_COLOR_MASK) | ((p4 & SECOND_COLOR_MASK) << 16); \
                            b = *(TileData01 + ((Y01 & 7) << 4) + ((X & 7) << 1)); \
                            uint32 p3 = COLORFUNC; \
                            p3 = (p3 & FIRST_THIRD_COLOR_MASK) | ((p3 & SECOND_COLOR_MASK) << 16); \
                            /* Xdel, Ydel: position (in 1/32nds) between the points */ \
                            uint32 Xdel = (xPos >> 3) & 0x1F; \
                            uint32 Ydel = (yPos >> 3) & 0x1F; \
                            uint32 XY = (Xdel*Ydel) >> 5; \
                            uint32 area1 = 0x20 + XY - Xdel - Ydel; \
                            uint32 area2 = Xdel - XY; \
                            uint32 area3 = Ydel - XY; \
                            uint32 area4 = XY; \
                            uint32 tempColor = ((area1 * p1) + \
                                                (area2 * p2) + \
                                                (area3 * p3) + \
                                                (area4 * p4)) >> 5; \
                            TYPE theColor = (tempColor & FIRST_THIRD_COLOR_MASK) | ((tempColor >> 16) & SECOND_COLOR_MASK); \
                            *p = (FUNC) | ALPHA_BITS_MASK; \
                            *d = GFX.Z1; \
                        } \
                    } \
                } \
                else \
                    /* The oversampling method: get the colors at four corners of a square */ \
                    /* in the _displayed_ image, and average them.  It's sharp and clean, but */ \
                    /* gives the usual huge pixels when the source image gets "close." */ \
                { \
                    /* Find the dimensions of the square in the source image whose corners will be examined. */ \
                    uint32 aaDelX = aa >> 1; \
                    uint32 ccDelX = cc >> 1; \
                    uint32 bbDelY = l->MatrixB >> 1; \
                    uint32 ddDelY = l->MatrixD >> 1; \
                    /* Offset the location within the source image so that the four sampled points */ \
                    /* center around where the single point would otherwise have been drawn. */ \
                    BB -= (bbDelY >> 1); \
                    DD -= (ddDelY >> 1); \
                    AA -= (aaDelX >> 1); \
                    CC -= (ccDelX >> 1); \
                    uint32 BB10 = BB + aaDelX; \
                    uint32 BB01 = BB + bbDelY; \
                    uint32 BB11 = BB + aaDelX + bbDelY; \
                    uint32 DD10 = DD + ccDelX; \
                    uint32 DD01 = DD + ddDelY; \
                    uint32 DD11 = DD + ccDelX + ddDelY; \
                    for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
                    { \
                        uint32 X = ((AA + BB) >> 8) & 0x3ff; \
                        uint32 Y = ((CC + DD) >> 8) & 0x3ff; \
                        uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
			uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
			GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                        if (GFX.Z1 > *d && b) \
                        { \
                            /* X, Y, X10, Y10, etc. are the coordinates of the four pixels within the */ \
                            /* source image that we're going to examine. */ \
                            uint32 X10 = ((AA + BB10) >> 8) & 0x3ff; \
                            uint32 Y10 = ((CC + DD10) >> 8) & 0x3ff; \
                            uint32 X01 = ((AA + BB01) >> 8) & 0x3ff; \
                            uint32 Y01 = ((CC + DD01) >> 8) & 0x3ff; \
                            uint32 X11 = ((AA + BB11) >> 8) & 0x3ff; \
                            uint32 Y11 = ((CC + DD11) >> 8) & 0x3ff; \
                            uint8 *TileData10 = VRAM1 + (Memory.VRAM[((Y10 & ~7) << 5) + ((X10 >> 2) & ~1)] << 7); \
                            uint8 *TileData01 = VRAM1 + (Memory.VRAM[((Y01 & ~7) << 5) + ((X01 >> 2) & ~1)] << 7); \
                            uint8 *TileData11 = VRAM1 + (Memory.VRAM[((Y11 & ~7) << 5) + ((X11 >> 2) & ~1)] << 7); \
                            TYPE p1 = COLORFUNC; \
                            b = *(TileData10 + ((Y10 & 7) << 4) + ((X10 & 7) << 1)); \
                            TYPE p2 = COLORFUNC; \
                            b = *(TileData01 + ((Y01 & 7) << 4) + ((X01 & 7) << 1)); \
                            TYPE p3 = COLORFUNC; \
                            b = *(TileData11 + ((Y11 & 7) << 4) + ((X11 & 7) << 1)); \
                            TYPE p4 = COLORFUNC; \
                            TYPE theColor = Q_INTERPOLATE(p1, p2, p3, p4); \
                            *p = (FUNC) | ALPHA_BITS_MASK; \
                            *d = GFX.Z1; \
                        } \
                    } \
                } \
            } \
            else \
            { \
                for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
                { \
                    uint32 xPos = AA + BB; \
                    uint32 xPix = xPos >> 8; \
                    uint32 yPos = CC + DD; \
                    uint32 yPix = yPos >> 8; \
                    uint32 X = xPix; \
                    uint32 Y = yPix; \
                    \
\
		    if(Settings.Dezaemon && PPU.Mode7Repeat == 2) \
		    { \
			X &= 0x7ff; \
			Y &= 0x7ff; \
		    } \
\
                    if (((X | Y) & ~0x3ff) == 0) \
                    { \
                        uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
			uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
			GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                        if (GFX.Z1 > *d && b) \
                        { \
                            /* X10 and Y01 are the X and Y coordinates of the next source point over. */ \
                            uint32 X10 = (xPix + dir) & 0x3ff; \
                            uint32 Y01 = (yPix + dir) & 0x3ff; \
                            uint8 *TileData10 = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X10 >> 2) & ~1)] << 7); \
                            uint8 *TileData11 = VRAM1 + (Memory.VRAM[((Y01 & ~7) << 5) + ((X10 >> 2) & ~1)] << 7); \
                            uint8 *TileData01 = VRAM1 + (Memory.VRAM[((Y01 & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
                            uint32 p1 = COLORFUNC; \
                            p1 = (p1 & FIRST_THIRD_COLOR_MASK) | ((p1 & SECOND_COLOR_MASK) << 16); \
                            b = *(TileData10 + ((Y & 7) << 4) + ((X10 & 7) << 1)); \
                            uint32 p2 = COLORFUNC; \
                            p2 = (p2 & FIRST_THIRD_COLOR_MASK) | ((p2 & SECOND_COLOR_MASK) << 16); \
                            b = *(TileData11 + ((Y01 & 7) << 4) + ((X10 & 7) << 1)); \
                            uint32 p4 = COLORFUNC; \
                            p4 = (p4 & FIRST_THIRD_COLOR_MASK) | ((p4 & SECOND_COLOR_MASK) << 16); \
                            b = *(TileData01 + ((Y01 & 7) << 4) + ((X & 7) << 1)); \
                            uint32 p3 = COLORFUNC; \
                            p3 = (p3 & FIRST_THIRD_COLOR_MASK) | ((p3 & SECOND_COLOR_MASK) << 16); \
                            /* Xdel, Ydel: position (in 1/32nds) between the points */ \
                            uint32 Xdel = (xPos >> 3) & 0x1F; \
                            uint32 Ydel = (yPos >> 3) & 0x1F; \
                            uint32 XY = (Xdel*Ydel) >> 5; \
                            uint32 area1 = 0x20 + XY - Xdel - Ydel; \
                            uint32 area2 = Xdel - XY; \
                            uint32 area3 = Ydel - XY; \
                            uint32 area4 = XY; \
                            uint32 tempColor = ((area1 * p1) + \
                                                (area2 * p2) + \
                                                (area3 * p3) + \
                                                (area4 * p4)) >> 5; \
                            TYPE theColor = (tempColor & FIRST_THIRD_COLOR_MASK) | ((tempColor >> 16) & SECOND_COLOR_MASK); \
                            *p = (FUNC) | ALPHA_BITS_MASK; \
                            *d = GFX.Z1; \
                        } \
                    } \
                    else \
                    { \
                        if (PPU.Mode7Repeat == 3) \
                        { \
                            X = (x + HOffset) & 7; \
                            Y = (yy + CentreY) & 7; \
			    uint32 b = *(VRAM1 + ((Y & 7) << 4) + ((X & 7) << 1)); \
			    GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                            if (GFX.Z1 > *d && b) \
                            { \
                                TYPE theColor = COLORFUNC; \
                                *p = (FUNC) | ALPHA_BITS_MASK; \
                                *d = GFX.Z1; \
                            } \
                        } \
                    } \
                } \
            } \
        } \
    }

STATIC uint32 Q_INTERPOLATE(uint32 A, uint32 B, uint32 C, uint32 D)
{
    register uint32 x = ((A >> 2) & HIGH_BITS_SHIFTED_TWO_MASK) +
                            ((B >> 2) & HIGH_BITS_SHIFTED_TWO_MASK) +
                            ((C >> 2) & HIGH_BITS_SHIFTED_TWO_MASK) +
                            ((D >> 2) & HIGH_BITS_SHIFTED_TWO_MASK);
    register uint32 y = (A & TWO_LOW_BITS_MASK) +
                            (B & TWO_LOW_BITS_MASK) +
                            (C & TWO_LOW_BITS_MASK) +
                            (D & TWO_LOW_BITS_MASK);
    y = (y>>2) & TWO_LOW_BITS_MASK;
    return x+y;
}

void DrawBGMode7Background16_i (uint8 *Screen, int bg)
{
    RENDER_BACKGROUND_MODE7_i (uint16, theColor, (GFX.ScreenColors[b & GFX.Mode7Mask]));
}

void DrawBGMode7Background16Add_i (uint8 *Screen, int bg)
{
    RENDER_BACKGROUND_MODE7_i (uint16, *(d + GFX.DepthDelta) ?
					(*(d + GFX.DepthDelta) != 1 ?
					    (COLOR_ADD (theColor,
						       p [GFX.Delta])) :
					    (COLOR_ADD (theColor,
						       GFX.FixedColour))) :
					 theColor, (GFX.ScreenColors[b & GFX.Mode7Mask]));
}

void DrawBGMode7Background16Add1_2_i (uint8 *Screen, int bg)
{
    RENDER_BACKGROUND_MODE7_i (uint16, *(d + GFX.DepthDelta) ?
					(*(d + GFX.DepthDelta) != 1 ?
					    COLOR_ADD1_2 (theColor,
						          p [GFX.Delta]) :
					    COLOR_ADD (theColor,
						       GFX.FixedColour)) :
					 theColor, (GFX.ScreenColors[b & GFX.Mode7Mask]));
}

void DrawBGMode7Background16Sub_i (uint8 *Screen, int bg)
{
    RENDER_BACKGROUND_MODE7_i (uint16, *(d + GFX.DepthDelta) ?
					(*(d + GFX.DepthDelta) != 1 ?
					    COLOR_SUB (theColor,
						       p [GFX.Delta]) :
					    COLOR_SUB (theColor,
						       GFX.FixedColour)) :
					 theColor, (GFX.ScreenColors[b & GFX.Mode7Mask]));
}

void DrawBGMode7Background16Sub1_2_i (uint8 *Screen, int bg)
{
    RENDER_BACKGROUND_MODE7_i (uint16, *(d + GFX.DepthDelta) ?
					(*(d + GFX.DepthDelta) != 1 ?
					    COLOR_SUB1_2 (theColor,
						       p [GFX.Delta]) :
					    COLOR_SUB (theColor,
						       GFX.FixedColour)) :
					 theColor, (GFX.ScreenColors[b & GFX.Mode7Mask]));
}
#endif

#define _BUILD_SETUP(F) \
GFX.BuildPixel = BuildPixel##F; \
GFX.BuildPixel2 = BuildPixel2##F; \
GFX.DecomposePixel = DecomposePixel##F; \
RED_LOW_BIT_MASK = RED_LOW_BIT_MASK_##F; \
GREEN_LOW_BIT_MASK = GREEN_LOW_BIT_MASK_##F; \
BLUE_LOW_BIT_MASK = BLUE_LOW_BIT_MASK_##F; \
RED_HI_BIT_MASK = RED_HI_BIT_MASK_##F; \
GREEN_HI_BIT_MASK = GREEN_HI_BIT_MASK_##F; \
BLUE_HI_BIT_MASK = BLUE_HI_BIT_MASK_##F; \
MAX_RED = MAX_RED_##F; \
MAX_GREEN = MAX_GREEN_##F; \
MAX_BLUE = MAX_BLUE_##F; \
GREEN_HI_BIT = ((MAX_GREEN_##F + 1) >> 1); \
SPARE_RGB_BIT_MASK = SPARE_RGB_BIT_MASK_##F; \
RGB_LOW_BITS_MASK = (RED_LOW_BIT_MASK_##F | \
 		     GREEN_LOW_BIT_MASK_##F | \
		     BLUE_LOW_BIT_MASK_##F); \
RGB_HI_BITS_MASK = (RED_HI_BIT_MASK_##F | \
		    GREEN_HI_BIT_MASK_##F | \
		    BLUE_HI_BIT_MASK_##F); \
RGB_HI_BITS_MASKx2 = ((RED_HI_BIT_MASK_##F | \
		       GREEN_HI_BIT_MASK_##F | \
		       BLUE_HI_BIT_MASK_##F) << 1); \
RGB_REMOVE_LOW_BITS_MASK = ~RGB_LOW_BITS_MASK; \
FIRST_COLOR_MASK = FIRST_COLOR_MASK_##F; \
SECOND_COLOR_MASK = SECOND_COLOR_MASK_##F; \
THIRD_COLOR_MASK = THIRD_COLOR_MASK_##F; \
ALPHA_BITS_MASK = ALPHA_BITS_MASK_##F; \
FIRST_THIRD_COLOR_MASK = FIRST_COLOR_MASK | THIRD_COLOR_MASK; \
TWO_LOW_BITS_MASK = RGB_LOW_BITS_MASK | (RGB_LOW_BITS_MASK << 1); \
HIGH_BITS_SHIFTED_TWO_MASK = (( (FIRST_COLOR_MASK | SECOND_COLOR_MASK | THIRD_COLOR_MASK) & \
                                ~TWO_LOW_BITS_MASK ) >> 2);

void RenderScreen (uint8 *Screen, bool8_32 sub, bool8_32 force_no_add, uint8 D)
{
    bool8_32 BG0;
    bool8_32 BG1;
    bool8_32 BG2;
    bool8_32 BG3;
    bool8_32 OB;
	uint8 BGMode = PPU.BGMode;
	struct SGFX * gfx = &GFX; 
    gfx->S = Screen;

    if (!sub)
    {
	gfx->pCurrentClip = &IPPU.Clip [0];
	BG0 = ON_MAIN (0);
	BG1 = ON_MAIN (1);
	BG2 = ON_MAIN (2);
	BG3 = ON_MAIN (3);
	OB  = ON_MAIN (4);
    }
    else
    {
	gfx->pCurrentClip = &IPPU.Clip [1];
	BG0 = ON_SUB (0);
	BG1 = ON_SUB (1);
	BG2 = ON_SUB (2);
	BG3 = ON_SUB (3);
	OB  = ON_SUB (4);
    }

    sub |= force_no_add;

    if (BGMode <= 1)
    {
	if (OB)
	{
	    SelectTileRenderer (sub || !SUB_OR_ADD(4));
	    DrawOBJS (!sub, D);
	}
	if (BG0)
	{
	    SelectTileRenderer (sub || !SUB_OR_ADD(0));
	    DrawBackground (BGMode, 0, D + 10, D + 14);
	}
	if (BG1)
	{
	    SelectTileRenderer (sub || !SUB_OR_ADD(1));
	    DrawBackground (BGMode, 1, D + 9, D + 13);
	}
	if (BG2)
	{
	    SelectTileRenderer (sub || !SUB_OR_ADD(2));
	    DrawBackground (BGMode, 2, D + 3, 
			    (Memory.FillRAM [0x2105] & 8) == 0 ? D + 6 : D + 17);
	}
	if (BG3 && BGMode == 0)
	{
	    SelectTileRenderer (sub || !SUB_OR_ADD(3));
	    DrawBackground (BGMode, 3, D + 2, D + 5);
	}
    }
    else if (BGMode != 7)
    {
	if (OB)
	{
	    SelectTileRenderer (sub || !SUB_OR_ADD(4));
	    DrawOBJS (!sub, D);
	}
	if (BG0)
	{
	    SelectTileRenderer (sub || !SUB_OR_ADD(0));
	    DrawBackground (BGMode, 0, D + 5, D + 13);
	}
	if (BGMode != 6 && BG1)
	{
	    SelectTileRenderer (sub || !SUB_OR_ADD(1));
	    DrawBackground (BGMode, 1, D + 2, D + 9);
	}
    }
    else
    {
	if (OB)
	{
	    SelectTileRenderer (sub || !SUB_OR_ADD(4));
	    DrawOBJS (!sub, D);
	}
	if (BG0 || ((Memory.FillRAM [0x2133] & 0x40) && BG1))
	{
	    int bg;

	    if (Memory.FillRAM [0x2133] & 0x40)
	    {
		gfx->Mode7Mask = 0x7f;
		gfx->Mode7PriorityMask = 0x80;
		Mode7Depths [0] = 5 + D;
		Mode7Depths [1] = 9 + D;
		bg = 1;
	    }
	    else
	    {
		gfx->Mode7Mask = 0xff;
		gfx->Mode7PriorityMask = 0;
		Mode7Depths [0] = 5 + D;
		Mode7Depths [1] = 5 + D;
		bg = 0;
	    }
	    if (sub || !SUB_OR_ADD(0))
	    {
#ifndef _ZAURUS
		if (!Settings.Mode7Interpolate)
#endif
		    DrawBGMode7Background16 (Screen, bg);
#ifndef _ZAURUS
		else
		    DrawBGMode7Background16_i (Screen, bg);
#endif
	    }
	    else
	    {
		if (gfx->r2131 & 0x80)
		{
		    if (gfx->r2131 & 0x40)
		    {
#ifndef _ZAURUS
			if (!Settings.Mode7Interpolate)
#endif
			    DrawBGMode7Background16Sub1_2 (Screen, bg);
#ifndef _ZAURUS
			else
			    DrawBGMode7Background16Sub1_2_i (Screen, bg);
#endif
		    }
		    else
		    {
#ifndef _ZAURUS
			if (!Settings.Mode7Interpolate)
#endif
			    DrawBGMode7Background16Sub (Screen, bg);
#ifndef _ZAURUS
			else
			    DrawBGMode7Background16Sub_i (Screen, bg);
#endif
		    }
		}
		else
		{
		    if (gfx->r2131 & 0x40)
		    {
#ifndef _ZAURUS
			if (!Settings.Mode7Interpolate)
#endif
			    DrawBGMode7Background16Add1_2 (Screen, bg);
#ifndef _ZAURUS
			else
			    DrawBGMode7Background16Add1_2_i (Screen, bg);
#endif
		    }
		    else
		    {
#ifndef _ZAURUS
			if (!Settings.Mode7Interpolate)
#endif
			    DrawBGMode7Background16Add (Screen, bg);
#ifndef _ZAURUS
			else
			    DrawBGMode7Background16Add_i (Screen, bg);
#endif
		    }
		}
	    }
	}
    }
}

#include "font.h"

void DisplayChar (uint8 *Screen, uint8 c, uint32 pitch)
{
    int line = (((c & 0x7f) - 32) >> 4) * font_height;
    int offset = (((c & 0x7f) - 32) & 15) * font_width;
#ifndef _ZAURUS
    if (Settings.SixteenBit)
#endif
    {
	int h, w;
	uint16 *s = (uint16 *) Screen;

	for (h = 0; h < font_height; h++, line++,
	     s += (pitch / 2) - font_width)
	{
	    for (w = 0; w < font_width; w++, s++)
	    {
		uint8 p = font [line][offset + w];

		if (p == '#')
		    *s = 0xffff;
		else
		if (p == '.')
		    *s = BLACK;
	    }
	}
    }
#ifndef _ZAURUS
    else
    {
	int h, w;
	uint8 *s = Screen;
	for (h = 0; h < font_height; h++, line++,
//	     s += GFX.PPL - font_width)
	     s += (pitch / 2) - font_width)
	{
	    for (w = 0; w < font_width; w++, s++)
	    {
		uint8 p = font [line][offset + w];

		if (p == '#')
		    *s = 255;
		else
		if (p == '.')
		    *s = BLACK;
	    }
	}
    }
#endif
}

void S9xDisplayFrameRate (uint8 *screen, uint32 pitch)
{
#ifdef PANDORA
// the pandora directly hands over the correct position for the string since it
// is resolution dependant!
    uint8 *Screen = screen;
#else
    uint8 *Screen = screen + 2 + 400 +
      (IPPU.RenderedScreenHeight - font_height - 1) * pitch;
#endif
    char string [16];
//    int len = 5;

    sprintf (string, "%02d/%02d", IPPU.DisplayedRenderedFrameCount,
	     (int) Memory.ROMFramesPerSecond);

    int i;
#ifdef _ZAURUS
    Screen += (font_width - 1) * sizeof(uint16);
#endif
    for (i = 0; i < 5; i++)
    {
	DisplayChar (Screen, string [i], pitch);
	Screen +=  (font_width - 1) * sizeof (uint16);
    }
}

void S9xDisplayString (const char *string, uint8 *screen, uint32 pitch, int ypos)
{
	uint8 *Screen = screen + 2 + (ypos - font_height * 5) * pitch;

	if(ypos == 0)
		Screen = screen + 2 + (IPPU.RenderedScreenHeight - font_height * 1) * pitch;

	int len = strlen (string);
	int max_chars = IPPU.RenderedScreenWidth / (font_width - 1);
	int char_count = 0;
	int i;

	for (i = 0; i < len; i++, char_count++)
	{
		if (char_count >= max_chars || string [i] < 32)
		{
		    Screen -= (font_width - 1) * sizeof (uint16) * max_chars;
		    Screen += font_height * pitch;
		    if (Screen >= GFX.Screen + GFX.Pitch * IPPU.RenderedScreenHeight)
				break;
		    char_count -= max_chars;
		}
		if (string [i] < 32)
		    continue;
		DisplayChar (Screen, string [i], pitch);
		Screen += (font_width - 1) * sizeof (uint16); 
	}
}

void S9xUpdateScreen () // ~30-50ms! (called from FLUSH_REDRAW())
{
    int32 x2 = 1;
	struct SGFX * gfx = &GFX; 
//	struct SPPU *ppu = &PPU;
	struct InternalPPU *ippu = &IPPU;

    gfx->S = gfx->Screen;

	unsigned char *memoryfillram = Memory.FillRAM;
    gfx->r2131 = memoryfillram [0x2131];
    gfx->r212c = memoryfillram [0x212c];
    gfx->r212d = memoryfillram [0x212d];
    gfx->r2130 = memoryfillram [0x2130];
	gfx->Pseudo = (memoryfillram [0x2133] & 8) != 0 &&
		 (gfx->r212c & 15) != (gfx->r212d & 15) &&
		 (gfx->r2131 & 0x3f) == 0;

    if (ippu->OBJChanged)
	{	
		S9xSetupOBJ ();
	}

    if (PPU.RecomputeClipWindows)
    {
		ComputeClipWindows ();
		PPU.RecomputeClipWindows = FALSE;
    }

    gfx->StartY = ippu->PreviousLine;
    if ((gfx->EndY = ippu->CurrentLine - 1) >= PPU.ScreenHeight)
		gfx->EndY = PPU.ScreenHeight - 1;

    uint32 starty = gfx->StartY;
    uint32 endy = gfx->EndY;

    if (Settings.SupportHiRes &&
	(PPU.BGMode == 5 || PPU.BGMode == 6 || ippu->LatchedInterlace))
    {
	if (PPU.BGMode == 5 || PPU.BGMode == 6)
	{
	    ippu->RenderedScreenWidth = 512;
	    x2 = 2;
	}
	if (ippu->LatchedInterlace)
	{
	    starty = gfx->StartY * 2;
	    endy = gfx->EndY * 2 + 1;
	}
	if (!ippu->DoubleWidthPixels)
	{
	    // The game has switched from lo-res to hi-res mode part way down
	    // the screen. Scale any existing lo-res pixels on screen
#ifndef _ZAURUS
		if (Settings.SixteenBit)
#endif
	    {
#if defined (USE_GLIDE) || defined (USE_OPENGL)
                if (
#ifdef USE_GLIDE
                    (Settings.GlideEnable && gfx->Pitch == 512) ||
#endif
#ifdef USE_OPENGL
                    (Settings.OpenGLEnable && gfx->Pitch == 512) ||
#endif
                    0)
		{
		    // Have to back out of the speed up hack where the low res.
		    // SNES image was rendered into a 256x239 sized buffer,
		    // ignoring the true, larger size of the buffer.
		    
		    for (int32 y = (int32) gfx->StartY - 1; y >= 0; y--)
		    {
				uint16 *p = (uint16 *) (gfx->Screen + y * gfx->Pitch) + 255;
				uint16 *q = (uint16 *) (gfx->Screen + y * gfx->RealPitch) + 510;
				for (int x = 255; x >= 0; x--, p--, q -= 2)
					*q = *(q + 1) = *p;
		    }
		    gfx->Pitch = gfx->Pitch2 = gfx->RealPitch;
                    gfx->PPL = gfx->Pitch >> 1;
                    gfx->PPLx2 = gfx->Pitch;
                    gfx->ZPitch = gfx->PPL;
		}
		else
#endif
		for (uint32 y = 0; y < gfx->StartY; y++)
		{
		    uint16 *p = (uint16 *) (gfx->Screen + y * gfx->Pitch) + 255;
		    uint16 *q = (uint16 *) (gfx->Screen + y * gfx->Pitch) + 510;
		    for (int x = 255; x >= 0; x--, p--, q -= 2)
			*q = *(q + 1) = *p;
		}
	    }
#ifndef _ZAURUS
	    else
	    {
			for (uint32 y = 0; y < gfx->StartY; y++)
			{
				uint8 *p = gfx->Screen + y * gfx->Pitch + 255;
				uint8 *q = gfx->Screen + y * gfx->Pitch + 510;
				for (int x = 255; x >= 0; x--, p--, q -= 2)
				*q = *(q + 1) = *p;
			}
	    }
#endif
	    ippu->DoubleWidthPixels = TRUE;
	}
    }



    uint32 black = BLACK | (BLACK << 16);
//#ifndef _ZAURUS
    if (Settings.Transparency && Settings.SixteenBit)
//#endif
    {
		if (gfx->Pseudo)
		{
			gfx->r2131 = 0x5f;
			gfx->r212d = (Memory.FillRAM [0x212c] ^
				 Memory.FillRAM [0x212d]) & 15;
			gfx->r212c &= ~gfx->r212d;
			gfx->r2130 |= 2;
		}

		if (!PPU.ForcedBlanking && ADD_OR_SUB_ON_ANYTHING &&
			(gfx->r2130 & 0x30) != 0x30 &&
			!((gfx->r2130 & 0x30) == 0x10 && ippu->Clip[1].Count[5] == 0))
		{
			struct ClipData *pClip;

			gfx->FixedColour = ((int)ippu->XB [PPU.FixedColourRed] << 11) | ((int)ippu->XB [PPU.FixedColourGreen] << 6) | (int)ippu->XB [PPU.FixedColourBlue];
//			gfx->FixedColour = BUILD_PIXEL (ippu->XB [PPU.FixedColourRed],
//					   ippu->XB [PPU.FixedColourGreen],
//					   ippu->XB [PPU.FixedColourBlue]);

	    // Clear the z-buffer, marking areas 'covered' by the fixed
	    // colour as depth 1.
	    pClip = &ippu->Clip [1];

	    // Clear the z-buffer
	    if (pClip->Count [5])
	    {

			// Colour window enabled.

			for (uint32 y = starty; y <= endy; y++)
			{

				ZeroMemory (gfx->SubZBuffer + y * gfx->ZPitch,
					ippu->RenderedScreenWidth);
				ZeroMemory (gfx->ZBuffer + y * gfx->ZPitch,
					ippu->RenderedScreenWidth);

				if (ippu->Clip [0].Count [5])
				{
					uint32 *p = (uint32 *) (gfx->SubScreen + y * gfx->Pitch2);
					uint32 *q = (uint32 *) ((uint16 *) p + ippu->RenderedScreenWidth);
					while (p < q) {
						*p++ = black;
						*p++ = black;
						*p++ = black;
						*p++ = black;
					}
				}

				
				for (uint32 c = 0; c < pClip->Count [5]; c++)
				{
					if (pClip->Right [c][5] > pClip->Left [c][5])
					{
						memset (gfx->SubZBuffer + y * gfx->ZPitch + pClip->Left [c][5] * x2,
							1, (pClip->Right [c][5] - pClip->Left [c][5]) * x2);
						if (ippu->Clip [0].Count [5])
						{
						// Blast, have to clear the sub-screen to the fixed-colour
						// because there is a colour window in effect clipping
						// the main screen that will allow the sub-screen
						// 'underneath' to show through.

						uint16 *p = (uint16 *) (gfx->SubScreen + y * gfx->Pitch2);
						uint16 *q = p + pClip->Right [c][5] * x2;
						p += pClip->Left [c][5] * x2;

						while (p < q) {
							*p++ = (uint16) gfx->FixedColour;
							*p++ = (uint16) gfx->FixedColour;
							*p++ = (uint16) gfx->FixedColour;
							*p++ = (uint16) gfx->FixedColour;
						}
						}
					}
				}
			}

	    }
	    else
	    {
		for (uint32 y = starty; y <= endy; y++)
		{
			ZeroMemory (gfx->ZBuffer + y * gfx->ZPitch,
				ippu->RenderedScreenWidth);
		    memset (gfx->SubZBuffer + y * gfx->ZPitch, 1,
			    ippu->RenderedScreenWidth);
		    if (ippu->Clip [0].Count [5])
		    {
			// Blast, have to clear the sub-screen to the fixed-colour
			// because there is a colour window in effect clipping
			// the main screen that will allow the sub-screen
			// 'underneath' to show through.

			uint32 b = gfx->FixedColour | (gfx->FixedColour << 16);
			register uint32 *p = (uint32 *) (gfx->SubScreen + y * gfx->Pitch2);
			uint32 *q = (uint32 *) ((uint16 *) p + ippu->RenderedScreenWidth);

			while (p < q) {
			    *p++ = b;
			    *p++ = b;
			    *p++ = b;
			    *p++ = b;
			}
		    }
		}

		}

	    if (ANYTHING_ON_SUB)
	    {
			gfx->DB = gfx->SubZBuffer;
			RenderScreen (gfx->SubScreen, TRUE, TRUE, SUB_SCREEN_DEPTH);
	    }

	    if (ippu->Clip [0].Count [5])
	    {
		if (strncmp (Memory.ROMId, "AQT", 3) != 0)
		{
		    // Have to copy the sub-screen to the main screen as there is
		    // a colour window in effect clipping the main screen allowing
		    // the sub-screen to show through.

		    if (ippu->Clip [1].Count [5])
		    {
			for (uint32 y = starty; y <= endy; y++)
			{
			    for (uint32 w = 0; w < ippu->Clip [1].Count [5]; w++)
			    {
				if (ippu->Clip [1].Right [w][5] >= ippu->Clip [1].Left [w][5])
				{
				    int offset = ippu->Clip [1].Left [w][5] * x2 * sizeof (uint16);
				    int width = (ippu->Clip [1].Right [w][5] - 
						 ippu->Clip [1].Left [w][5]) * x2 * sizeof (uint16);
				    memmove (gfx->Screen + y * gfx->Pitch2 + offset,
					     gfx->SubScreen + y * gfx->Pitch2 + offset,
					     width);
				}
			    }
			}
		    }
		    else
		    {
			for (uint32 y = starty; y <= endy; y++)
			    memmove (gfx->Screen + y * gfx->Pitch2,
				     gfx->SubScreen + y * gfx->Pitch2,
				     ippu->RenderedScreenWidth * sizeof (uint16));
		    }
		}
		else
		{
		    // Clear the areas 'outside' the colour window to black
		    // For now just clear all of the scanlines
		    for (uint32 y = starty; y <= endy; y++)
            {
                register uint32 *p = (uint32 *) (gfx->Screen + y * gfx->Pitch2);
                uint32 *q = (uint32 *) ((uint16 *) p + ippu->RenderedScreenWidth);

                while (p < q) {
                    *p++ = black;
                    *p++ = black;
                    *p++ = black;
                    *p++ = black;
				}
            }		    
		}
	    }

	    gfx->DB = gfx->ZBuffer;
	    RenderScreen (gfx->Screen, FALSE, FALSE, MAIN_SCREEN_DEPTH);
	    if (SUB_OR_ADD(5))
	    {
		uint32 back = ippu->ScreenColors [0];
		uint32 Left = 0;
		uint32 Right = 256;
		uint32 Count;

		pClip = &ippu->Clip [0];

		for (uint32 y = starty; y <= endy; y++) {
		    if (!(Count = pClip->Count [5])) {
				Left = 0;
				Right = 256 * x2;
				Count = 1;
		    }

		    for (uint32 b = 0; b < Count; b++) {
				if (pClip->Count [5]) {
				    Left = pClip->Left [b][5] * x2;
				    Right = pClip->Right [b][5] * x2;
				    if (Right <= Left)
						continue;
			}

			if (gfx->r2131 & 0x80) {
			    if (gfx->r2131 & 0x40) {
					// Subtract, halving the result.
					register uint16 *p = (uint16 *) (gfx->Screen + y * gfx->Pitch2) + Left;
					register uint8 *d = gfx->ZBuffer + y * gfx->ZPitch;
					register uint8 *s = gfx->SubZBuffer + y * gfx->ZPitch + Left;
					register uint8 *e = d + Right;
					uint16 back_fixed = COLOR_SUB (back, gfx->FixedColour);

					d += Left;
					while (d < e) {
					    if (*d == 0) {
							if (*s) {
							    if (*s != 1)
									*p = COLOR_SUB1_2 (back, *(p + gfx->Delta));
							    else
									*p = back_fixed;
							} else
							    *p = (uint16) back;
					    }
					    d++;
					    p++;
					    s++;
					}
			    } else {
					// Subtract
					register uint16 *p = (uint16 *) (gfx->Screen + y * gfx->Pitch2) + Left;
					register uint8 *d = gfx->ZBuffer + y * gfx->ZPitch;
					register uint8 *s = gfx->SubZBuffer + y * gfx->ZPitch + Left;
					register uint8 *e = d + Right;
					uint16 back_fixed = COLOR_SUB (back, gfx->FixedColour);

					d += Left;
					while (d < e) {
					    if (*d == 0) {
							if (*s) {
							    if (*s != 1)
									*p = COLOR_SUB (back, *(p + gfx->Delta));
							    else
									*p = back_fixed;
							} else
							    *p = (uint16) back;
					    }
					    d++;
					    p++;
					    s++;
					}
			    }
			} else
			if (gfx->r2131 & 0x40) {
				register uint16 *p = (uint16 *) (gfx->Screen + y * gfx->Pitch2) + Left;
				register uint8 *d = gfx->ZBuffer + y * gfx->ZPitch;
				register uint8 *s = gfx->SubZBuffer + y * gfx->ZPitch + Left;
				register uint8 *e = d + Right;
			    uint16 back_fixed = COLOR_ADD (back, gfx->FixedColour);

			    d += Left;
			    while (d < e) {
					if (*d == 0) {
					    if (*s) {
							if (*s != 1)
							    *p = COLOR_ADD1_2 (back, *(p + gfx->Delta));
							else
							    *p = back_fixed;
					    } else
							*p = (uint16) back;
					}
					d++;
					p++;
					s++;
			    }
			}
			else
			if (back)
			{
				register uint16 *p = (uint16 *) (gfx->Screen + y * gfx->Pitch2) + Left;
				register uint8 *d = gfx->ZBuffer + y * gfx->ZPitch;
				register uint8 *s = gfx->SubZBuffer + y * gfx->ZPitch + Left;
				register uint8 *e = d + Right;
			    uint16 back_fixed = COLOR_ADD (back, gfx->FixedColour);

			    d += Left;
			    while (d < e) {
					if (*d == 0) {
					    if (*s) {
							if (*s != 1)
							    *p = COLOR_ADD (back, *(p + gfx->Delta));
							else	
							    *p = back_fixed;
					    } else
							*p = (uint16) back;
					}
					d++;
					p++;
					s++;
			    }
			}
			else
			{
			    if (!pClip->Count [5]) {
					// The backdrop has not been cleared yet - so
					// copy the sub-screen to the main screen
					// or fill it with the back-drop colour if the
					// sub-screen is clear.
					register uint16 *p = (uint16 *) (gfx->Screen + y * gfx->Pitch2) + Left;
					register uint8 *d = gfx->ZBuffer + y * gfx->ZPitch;
					register uint8 *s = gfx->SubZBuffer + y * gfx->ZPitch + Left;
					register uint8 *e = d + Right;

					d += Left;
					while (d < e) {
					    if (*d == 0) {
							if (*s) {
								if (*s != 1)
									*p = *(p + gfx->Delta);
								else	
									*p = gfx->FixedColour;
							} else
								*p = (uint16) back;
					    }
					    d++;
					    p++;
					    s++;
					}
				}
			}
		    }
		}

	    } else {
			// Subscreen not being added to back
			uint32 back = ippu->ScreenColors [0] | (ippu->ScreenColors [0] << 16);
			pClip = &ippu->Clip [0];

			if (pClip->Count [5]) {
				for (uint32 y = starty; y <= endy; y++) {
					for (uint32 b = 0; b < pClip->Count [5]; b++) {
						uint32 Left = pClip->Left [b][5] * x2;
						uint32 Right = pClip->Right [b][5] * x2;
						register uint16 *p = (uint16 *) (gfx->Screen + y * gfx->Pitch2) + Left;
						register uint8 *d = gfx->ZBuffer + y * gfx->ZPitch;
						register uint8 *e = d + Right;
						d += Left;

						while (d < e) {
							if (*d++ == 0)
								*p = (int16) back;
							p++;
						}
					}
				}
			} else {
				for (uint32 y = starty; y <= endy; y++) {
					register uint16 *p = (uint16 *) (gfx->Screen + y * gfx->Pitch2);
					register uint8 *d = gfx->ZBuffer + y * gfx->ZPitch;
					register uint8 *e = d + 256 * x2;

					while (d < e) {
						if (*d++ == 0)
							*p = (int16) back;
						p++;
					}
				}
			}
	    }
	} else {
	    // 16bit and transparency but currently no transparency effects in
	    // operation.

	    uint32 back = ippu->ScreenColors [0] | 
			 (ippu->ScreenColors [0] << 16);

	    if (PPU.ForcedBlanking)
			back = black;
	    if (ippu->Clip [0].Count[5]) {
			for (uint32 y = starty; y <= endy; y++) {
				register uint32 *p = (uint32 *) (gfx->Screen + y * gfx->Pitch2);
				uint32 *q = (uint32 *) ((uint16 *) p + ippu->RenderedScreenWidth);

				while (p < q) {
					*p++ = black;
					*p++ = black;
					*p++ = black;
					*p++ = black;
				}

				for (uint32 c = 0; c < ippu->Clip [0].Count [5]; c++) {
					if (ippu->Clip [0].Right [c][5] > ippu->Clip [0].Left [c][5]) {
						register uint16 *p = (uint16 *) (gfx->Screen + y * gfx->Pitch2);
						uint16 *q = p + ippu->Clip [0].Right [c][5] * x2;
						p += ippu->Clip [0].Left [c][5] * x2;

						while (p < q) {
							*p++ = (uint16) back;
							*p++ = (uint16) back;
							*p++ = (uint16) back;
							*p++ = (uint16) back;
						}
					}
				}
			}
	    } else {
			for (uint32 y = starty; y <= endy; y++) {
				register uint32 *p = (uint32 *) (gfx->Screen + y * gfx->Pitch2);
				uint32 *q = (uint32 *) ((uint16 *) p + ippu->RenderedScreenWidth);
				while (p < q) {
					*p++ = back;
					*p++ = back;
					*p++ = back;
					*p++ = back;
				}
			}
	    }
	    if (!PPU.ForcedBlanking) {
			for (uint32 y = starty; y <= endy; y++) {
				ZeroMemory (gfx->ZBuffer + y * gfx->ZPitch,
					ippu->RenderedScreenWidth);
			}
			gfx->DB = gfx->ZBuffer;
			RenderScreen (gfx->Screen, FALSE, TRUE, SUB_SCREEN_DEPTH);
	    }
	}
    }
//#ifndef _ZAURUS
    else
    {
//	if (Settings.SixteenBit)
	{
	    uint32 back = ippu->ScreenColors [0] | (ippu->ScreenColors [0] << 16);
	    if (PPU.ForcedBlanking)
		back = black;
	    else
		{
			SelectTileRenderer (TRUE);
		}
	    for (uint32 y = starty; y <= endy; y++)
	    {
			register uint32 *p = (uint32 *) (gfx->Screen + y * gfx->Pitch2);
			uint32 *q = (uint32 *) ((uint16 *) p + ippu->RenderedScreenWidth);
			while (p < q) {
				*p++ = back;
				*p++ = back;
				*p++ = back;
				*p++ = back;
			}
	    }
	}
/*	else
	{
	    for (uint32 y = starty; y <= endy; y++)
	    {
		ZeroMemory (gfx->Screen + y * gfx->Pitch2,
			    ippu->RenderedScreenWidth);
	    }
	}
*/	if (!PPU.ForcedBlanking)
	{
	    for (uint32 y = starty; y <= endy; y++)
	    {
		ZeroMemory (gfx->ZBuffer + y * gfx->ZPitch,
			    ippu->RenderedScreenWidth);
	    }
	    gfx->DB = gfx->ZBuffer;
	    gfx->pCurrentClip = &ippu->Clip [0];

#define FIXCLIP(n)\
if (gfx->r212c & (1 << (n))) \
    gfx->pCurrentClip = &ippu->Clip [0]; \
else \
    gfx->pCurrentClip = &ippu->Clip [1]


#define DISPLAY(n)\
    (!(PPU.BG_Forced & n) && \
      (gfx->r212c & n) || \
     ((gfx->r212d & n) && subadd))

	    uint8 subadd = gfx->r2131 & 0x3f;

	    bool8_32 BG0 = DISPLAY(1);
	    bool8_32 BG1 = DISPLAY(2);
	    bool8_32 BG2 = DISPLAY(4);
	    bool8_32 BG3 = DISPLAY(8);
	    bool8_32 OB  = DISPLAY(16);

	    if (PPU.BGMode <= 1)
	    {
		if (OB)
		{
		    FIXCLIP(4);
		    DrawOBJS ();
		}
		if (BG0)
		{
		    FIXCLIP(0);
		    DrawBackground (PPU.BGMode, 0, 10, 14);
		}
		if (BG1)
		{
		    FIXCLIP(1);
		    DrawBackground (PPU.BGMode, 1, 9, 13);
		}
		if (BG2)
		{
		    FIXCLIP(2);
		    DrawBackground (PPU.BGMode, 2, 3,
				    (Memory.FillRAM [0x2105] & 8) == 0 ? 6 : 17);
		}
		if (BG3 && PPU.BGMode == 0)
		{
		    FIXCLIP(3);
		    DrawBackground (PPU.BGMode, 3, 2, 5);
		}
	    }
	    else if (PPU.BGMode != 7)
	    {
		if (OB)
		{
		    FIXCLIP(4);
		    DrawOBJS ();
		}
		if (BG0)
		{
		    FIXCLIP(0);
		    DrawBackground (PPU.BGMode, 0, 5, 13);
		}
		if (BG1 && PPU.BGMode != 6)
		{
		    FIXCLIP(1);
		    DrawBackground (PPU.BGMode, 1, 2, 9);
		}
	    }
	    else
	    {
		if (OB)
		{
		    FIXCLIP(4);
		    DrawOBJS ();
		}
		if (BG0 || ((Memory.FillRAM [0x2133] & 0x40) && BG1))
		{
		    int bg;
		    FIXCLIP(0);
		    if (Memory.FillRAM [0x2133] & 0x40)
		    {
			gfx->Mode7Mask = 0x7f;
			gfx->Mode7PriorityMask = 0x80;
			Mode7Depths [0] = 5;
			Mode7Depths [1] = 9;
			bg = 1;
		    }
		    else
		    {
			gfx->Mode7Mask = 0xff;
			gfx->Mode7PriorityMask = 0;
			Mode7Depths [0] = 5;
			Mode7Depths [1] = 5;
			bg = 0;
		    }
/*
		    if (!Settings.SixteenBit)
				DrawBGMode7Background (gfx->Screen, bg);
		    else
		    {
				if (!Settings.Mode7Interpolate)
				{	
*/				    DrawBGMode7Background16 (gfx->Screen, bg);
/*				}
				else
				{	
				    DrawBGMode7Background16_i (gfx->Screen, bg);
				}
		  	}
*/		}
	    }
	}
    }
//#endif
    if (Settings.SupportHiRes && PPU.BGMode != 5 && PPU.BGMode != 6)
    {
	if (ippu->DoubleWidthPixels)
	{
	    // Mixure of background modes used on screen - scale width
	    // of all non-mode 5 and 6 pixels.
#ifndef _ZAURUS
		if (Settings.SixteenBit)
	    {
#endif
		for (register uint32 y = gfx->StartY; y <= gfx->EndY; y++)
		{
		    register uint16 *p = (uint16 *) (gfx->Screen + y * gfx->Pitch) + 255;
		    register uint16 *q = (uint16 *) (gfx->Screen + y * gfx->Pitch) + 510;
		    for (register int x = 255; x >= 0; x--, p--, q -= 2)
			*q = *(q + 1) = *p;
		}
#ifndef _ZAURUS
	    }
	    else
	    {
		for (register uint32 y = gfx->StartY; y <= gfx->EndY; y++)
		{
		    register uint8 *p = gfx->Screen + y * gfx->Pitch + 255;
		    register uint8 *q = gfx->Screen + y * gfx->Pitch + 510;
		    for (register int x = 255; x >= 0; x--, p--, q -= 2)
			*q = *(q + 1) = *p;
		}
	    }
#endif
	}

	if (ippu->LatchedInterlace)
	{
	    // Interlace is enabled - double the height of all non-mode 5 and 6
	    // pixels.
	    for (uint32 y = gfx->StartY; y <= gfx->EndY; y++)
	    {
		memmove (gfx->Screen + (y * 2 + 1) * gfx->Pitch2,
			 gfx->Screen + y * 2 * gfx->Pitch2,
			 gfx->Pitch2);
	    }
	}

    }
    ippu->PreviousLine = ippu->CurrentLine;
}

#ifdef GFX_MULTI_FORMAT

#define _BUILD_PIXEL(F) \
uint32 BuildPixel##F(uint32 R, uint32 G, uint32 B) \
{ \
    return (BUILD_PIXEL_##F(R,G,B)); \
}\
uint32 BuildPixel2##F(uint32 R, uint32 G, uint32 B) \
{ \
    return (BUILD_PIXEL2_##F(R,G,B)); \
} \
void DecomposePixel##F(uint32 pixel, uint32 &R, uint32 &G, uint32 &B) \
{ \
    DECOMPOSE_PIXEL_##F(pixel,R,G,B); \
}

_BUILD_PIXEL(RGB565)
_BUILD_PIXEL(RGB555)
_BUILD_PIXEL(BGR565)
_BUILD_PIXEL(BGR555)
_BUILD_PIXEL(GBR565)
_BUILD_PIXEL(GBR555)
_BUILD_PIXEL(RGB5551)

bool8_32 S9xSetRenderPixelFormat (int format)
{
    extern uint32 current_graphic_format;

    current_graphic_format = format;

    switch (format)
    {
    case RGB565:
	_BUILD_SETUP(RGB565)
	return (TRUE);
    case RGB555:
	_BUILD_SETUP(RGB555)
	return (TRUE);
    case BGR565:
	_BUILD_SETUP(BGR565)
	return (TRUE);
    case BGR555:
	_BUILD_SETUP(BGR555)
	return (TRUE);
    case GBR565:
	_BUILD_SETUP(GBR565)
	return (TRUE);
    case GBR555:
	_BUILD_SETUP(GBR555)
	return (TRUE);
    case RGB5551:
        _BUILD_SETUP(RGB5551)
        return (TRUE);
    default:
	break;
    }
    return (FALSE);
}
#endif
