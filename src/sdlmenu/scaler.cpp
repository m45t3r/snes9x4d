#include <cstddef>

#include "scaler.h"

#define AVERAGE(z, x) ((((z) & 0xF7DEF7DE) >> 1) + (((x) & 0xF7DEF7DE) >> 1))
#define AVERAGEHI(AB) ((((AB) & 0xF7DE0000) >> 1) + (((AB) & 0xF7DE) << 15))
#define AVERAGELO(CD) ((((CD) & 0xF7DE) >> 1) + (((CD) & 0xF7DE0000) >> 17))

void (*upscale_p)(uint32_t *dst, uint32_t *src, int width) = NULL;

/*
 * Approximately bilinear scaler, 256x224 to 320x240
 *
 * Copyright (C) 2014 hi-ban, Nebuleon <nebuleon.fumika@gmail.com>
 *
 * This function and all auxiliary functions are free software; you can
 * redistribute them and/or modify them under the terms of the GNU Lesser
 * General Public License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * These functions are distributed in the hope that they will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// Support math
#define Half(A) (((A) >> 1) & 0x7BEF)
#define Quarter(A) (((A) >> 2) & 0x39E7)
// Error correction expressions to piece back the lower bits together
#define RestHalf(A) ((A) & 0x0821)
#define RestQuarter(A) ((A) & 0x1863)

// Error correction expressions for quarters of pixels
#define Corr1_3(A, B)     Quarter(RestQuarter(A) + (RestHalf(B) << 1) + RestQuarter(B))
#define Corr3_1(A, B)     Quarter((RestHalf(A) << 1) + RestQuarter(A) + RestQuarter(B))

// Error correction expressions for halves
#define Corr1_1(A, B)     ((A) & (B) & 0x0821)

// Quarters
#define Weight1_3(A, B)   (Quarter(A) + Half(B) + Quarter(B) + Corr1_3(A, B))
#define Weight3_1(A, B)   (Half(A) + Quarter(A) + Quarter(B) + Corr3_1(A, B))

// Halves
#define Weight1_1(A, B)   (Half(A) + Half(B) + Corr1_1(A, B))

/* Upscales a 256x224 image to 320x240 using an approximate bilinear
 * resampling algorithm that only uses integer math.
 *
 * Input:
 *   src: A packed 256x224 pixel image. The pixel format of this image is
 *     RGB 565.
 *   width: The width of the source image. Should always be 256.
 * Output:
 *   dst: A packed 320x240 pixel image. The pixel format of this image is
 *     RGB 565.
 */
void upscale_256x224_to_320x240_bilinearish(uint32_t* dst, uint32_t* src, int width)
{
	uint16_t* Src16 = (uint16_t*) src;
	uint16_t* Dst16 = (uint16_t*) dst;
	// There are 64 blocks of 4 pixels horizontally, and 14 of 16 vertically.
	// Each block of 4x16 becomes 5x17.
	uint32_t BlockX, BlockY;
	uint16_t* BlockSrc;
	uint16_t* BlockDst;
	for (BlockY = 0; BlockY < 14; BlockY++)
	{
		BlockSrc = Src16 + BlockY * 256 * 16;
		BlockDst = Dst16 + BlockY * 320 * 17;
		for (BlockX = 0; BlockX < 64; BlockX++)
		{
			/* Horizontally:
			 * Before(4):
			 * (a)(b)(c)(d)
			 * After(5):
			 * (a)(abbb)(bc)(cccd)(d)
			 *
			 * Vertically:
			 * Before(16): After(17):
			 * (a)       (a)
			 * (b)       (b)
			 * (c)       (c)
			 * (d)       (cddd)
			 * (e)       (deee)
			 * (f)       (efff)
			 * (g)       (fggg)
			 * (h)       (ghhh)
			 * (i)       (hi)
			 * (j)       (iiij)
			 * (k)       (jjjk)
			 * (l)       (kkkl)
			 * (m)       (lllm)
			 * (n)       (mmmn)
			 * (o)       (n)
			 * (p)       (o)
			 *           (p)
			 */

			// -- Row 1 --
			uint16_t  _1 = *(BlockSrc               );
			*(BlockDst               ) = _1;
			uint16_t  _2 = *(BlockSrc            + 1);
			*(BlockDst            + 1) = Weight1_3( _1,  _2);
			uint16_t  _3 = *(BlockSrc            + 2);
			*(BlockDst            + 2) = Weight1_1( _2,  _3);
			uint16_t  _4 = *(BlockSrc            + 3);
			*(BlockDst            + 3) = Weight3_1( _3,  _4);
			*(BlockDst            + 4) = _4;

			// -- Row 2 --
			uint16_t  _5 = *(BlockSrc + 256 *  1    );
			*(BlockDst + 320 *  1    ) = _5;
			uint16_t  _6 = *(BlockSrc + 256 *  1 + 1);
			*(BlockDst + 320 *  1 + 1) = Weight1_3( _5,  _6);
			uint16_t  _7 = *(BlockSrc + 256 *  1 + 2);
			*(BlockDst + 320 *  1 + 2) = Weight1_1( _6,  _7);
			uint16_t  _8 = *(BlockSrc + 256 *  1 + 3);
			*(BlockDst + 320 *  1 + 3) = Weight3_1( _7,  _8);
			*(BlockDst + 320 *  1 + 4) = _8;

			// -- Row 3 --
			uint16_t  _9 = *(BlockSrc + 256 *  2    );
			*(BlockDst + 320 *  2    ) = _9;
			uint16_t  _10 = *(BlockSrc + 256 *  2 + 1);
			*(BlockDst + 320 *  2 + 1) = Weight1_3( _9, _10);
			uint16_t  _11 = *(BlockSrc + 256 *  2 + 2);
			*(BlockDst + 320 *  2 + 2) = Weight1_1(_10, _11);
			uint16_t  _12 = *(BlockSrc + 256 *  2 + 3);
			*(BlockDst + 320 *  2 + 3) = Weight3_1(_11, _12);
			*(BlockDst + 320 *  2 + 4) = _12;

			// -- Row 4 --
			uint16_t _13 = *(BlockSrc + 256 *  3    );
			*(BlockDst + 320 *  3    ) = Weight1_3( _9, _13);
			uint16_t _14 = *(BlockSrc + 256 *  3 + 1);
			*(BlockDst + 320 *  3 + 1) = Weight1_3(Weight1_3( _9, _10), Weight1_3(_13, _14));
			uint16_t _15 = *(BlockSrc + 256 *  3 + 2);
			*(BlockDst + 320 *  3 + 2) = Weight1_3(Weight1_1(_10, _11), Weight1_1(_14, _15));
			uint16_t _16 = *(BlockSrc + 256 *  3 + 3);
			*(BlockDst + 320 *  3 + 3) = Weight1_3(Weight3_1(_11, _12), Weight3_1(_15, _16));
			*(BlockDst + 320 *  3 + 4) = Weight1_3(_12, _16);

			// -- Row 5 --
			uint16_t _17 = *(BlockSrc + 256 *  4    );
			*(BlockDst + 320 *  4    ) = Weight1_3(_13, _17);
			uint16_t _18 = *(BlockSrc + 256 *  4 + 1);
			*(BlockDst + 320 *  4 + 1) = Weight1_3(Weight1_3(_13, _14), Weight1_3(_17, _18));
			uint16_t _19 = *(BlockSrc + 256 *  4 + 2);
			*(BlockDst + 320 *  4 + 2) = Weight1_3(Weight1_1(_14, _15), Weight1_1(_18, _19));
			uint16_t _20 = *(BlockSrc + 256 *  4 + 3);
			*(BlockDst + 320 *  4 + 3) = Weight1_3(Weight3_1(_15, _16), Weight3_1(_19, _20));
			*(BlockDst + 320 *  4 + 4) = Weight1_3(_16, _20);

			// -- Row 6 --
			uint16_t _21 = *(BlockSrc + 256 *  5    );
			*(BlockDst + 320 *  5    ) = Weight1_3(_17, _21);
			uint16_t _22 = *(BlockSrc + 256 *  5 + 1);
			*(BlockDst + 320 *  5 + 1) = Weight1_3(Weight1_3(_17, _18), Weight1_3(_21, _22));
			uint16_t _23 = *(BlockSrc + 256 *  5 + 2);
			*(BlockDst + 320 *  5 + 2) = Weight1_3(Weight1_1(_18, _19), Weight1_1(_22, _23));
			uint16_t _24 = *(BlockSrc + 256 *  5 + 3);
			*(BlockDst + 320 *  5 + 3) = Weight1_3(Weight3_1(_19, _20), Weight3_1(_23, _24));
			*(BlockDst + 320 *  5 + 4) = Weight1_3(_20, _24);

			// -- Row 7 --
			uint16_t _25 = *(BlockSrc + 256 *  6    );
			*(BlockDst + 320 *  6    ) = Weight1_3(_21, _25);
			uint16_t _26 = *(BlockSrc + 256 *  6 + 1);
			*(BlockDst + 320 *  6 + 1) = Weight1_3(Weight1_3(_21, _22), Weight1_3(_25, _26));
			uint16_t _27 = *(BlockSrc + 256 *  6 + 2);
			*(BlockDst + 320 *  6 + 2) = Weight1_3(Weight1_1(_22, _23), Weight1_1(_26, _27));
			uint16_t _28 = *(BlockSrc + 256 *  6 + 3);
			*(BlockDst + 320 *  6 + 3) = Weight1_3(Weight3_1(_23, _24), Weight3_1(_27, _28));
			*(BlockDst + 320 *  6 + 4) = Weight1_3(_24, _28);

			// -- Row 8 --
			uint16_t _29 = *(BlockSrc + 256 *  7    );
			*(BlockDst + 320 *  7    ) = Weight1_3(_25, _29);
			uint16_t _30 = *(BlockSrc + 256 *  7 + 1);
			*(BlockDst + 320 *  7 + 1) = Weight1_3(Weight1_3(_25, _26), Weight1_3(_29, _30));
			uint16_t _31 = *(BlockSrc + 256 *  7 + 2);
			*(BlockDst + 320 *  7 + 2) = Weight1_3(Weight1_1(_26, _27), Weight1_1(_30, _31));
			uint16_t _32 = *(BlockSrc + 256 *  7 + 3);
			*(BlockDst + 320 *  7 + 3) = Weight1_3(Weight3_1(_27, _28), Weight3_1(_31, _32));
			*(BlockDst + 320 *  7 + 4) = Weight1_3(_28, _32);

			// -- Row 9 --
			uint16_t _33 = *(BlockSrc + 256 *  8    );
			*(BlockDst + 320 *  8    ) = Weight1_1(_29, _33);
			uint16_t _34 = *(BlockSrc + 256 *  8 + 1);
			*(BlockDst + 320 *  8 + 1) = Weight1_1(Weight1_3(_29, _30), Weight1_3(_33, _34));
			uint16_t _35 = *(BlockSrc + 256 *  8 + 2);
			*(BlockDst + 320 *  8 + 2) = Weight1_1(Weight1_1(_30, _31), Weight1_1(_34, _35));
			uint16_t _36 = *(BlockSrc + 256 *  8 + 3);
			*(BlockDst + 320 *  8 + 3) = Weight1_1(Weight3_1(_31, _32), Weight3_1(_35, _36));
			*(BlockDst + 320 *  8 + 4) = Weight1_1(_32, _36);

			// -- Row 10 --
			uint16_t _37 = *(BlockSrc + 256 *  9    );
			*(BlockDst + 320 *  9    ) = Weight3_1(_33, _37);
			uint16_t _38 = *(BlockSrc + 256 *  9 + 1);
			*(BlockDst + 320 *  9 + 1) = Weight3_1(Weight1_3(_33, _34), Weight1_3(_37, _38));
			uint16_t _39 = *(BlockSrc + 256 *  9 + 2);
			*(BlockDst + 320 *  9 + 2) = Weight3_1(Weight1_1(_34, _35), Weight1_1(_38, _39));
			uint16_t _40 = *(BlockSrc + 256 *  9 + 3);
			*(BlockDst + 320 *  9 + 3) = Weight3_1(Weight3_1(_35, _36), Weight3_1(_39, _40));
			*(BlockDst + 320 *  9 + 4) = Weight3_1(_36, _40);

			// -- Row 11 --
			uint16_t _41 = *(BlockSrc + 256 * 10    );
			*(BlockDst + 320 * 10    ) = Weight3_1(_37, _41);
			uint16_t _42 = *(BlockSrc + 256 * 10 + 1);
			*(BlockDst + 320 * 10 + 1) = Weight3_1(Weight1_3(_37, _38), Weight1_3(_41, _42));
			uint16_t _43 = *(BlockSrc + 256 * 10 + 2);
			*(BlockDst + 320 * 10 + 2) = Weight3_1(Weight1_1(_38, _39), Weight1_1(_42, _43));
			uint16_t _44 = *(BlockSrc + 256 * 10 + 3);
			*(BlockDst + 320 * 10 + 3) = Weight3_1(Weight3_1(_39, _40), Weight3_1(_43, _44));
			*(BlockDst + 320 * 10 + 4) = Weight3_1(_40, _44);

			// -- Row 12 --
			uint16_t _45 = *(BlockSrc + 256 * 11    );
			*(BlockDst + 320 * 11    ) = Weight3_1(_41, _45);
			uint16_t _46 = *(BlockSrc + 256 * 11 + 1);
			*(BlockDst + 320 * 11 + 1) = Weight3_1(Weight1_3(_41, _42), Weight1_3(_45, _46));
			uint16_t _47 = *(BlockSrc + 256 * 11 + 2);
			*(BlockDst + 320 * 11 + 2) = Weight3_1(Weight1_1(_42, _43), Weight1_1(_46, _47));
			uint16_t _48 = *(BlockSrc + 256 * 11 + 3);
			*(BlockDst + 320 * 11 + 3) = Weight3_1(Weight3_1(_43, _44), Weight3_1(_47, _48));
			*(BlockDst + 320 * 11 + 4) = Weight3_1(_44, _48);

			// -- Row 13 --
			uint16_t _49 = *(BlockSrc + 256 * 12    );
			*(BlockDst + 320 * 12    ) = Weight3_1(_45, _49);
			uint16_t _50 = *(BlockSrc + 256 * 12 + 1);
			*(BlockDst + 320 * 12 + 1) = Weight3_1(Weight1_3(_45, _46), Weight1_3(_49, _50));
			uint16_t _51 = *(BlockSrc + 256 * 12 + 2);
			*(BlockDst + 320 * 12 + 2) = Weight3_1(Weight1_1(_46, _47), Weight1_1(_50, _51));
			uint16_t _52 = *(BlockSrc + 256 * 12 + 3);
			*(BlockDst + 320 * 12 + 3) = Weight3_1(Weight3_1(_47, _48), Weight3_1(_51, _52));
			*(BlockDst + 320 * 12 + 4) = Weight3_1(_48, _52);

			// -- Row 14 --
			uint16_t _53 = *(BlockSrc + 256 * 13    );
			*(BlockDst + 320 * 13    ) = Weight3_1(_49, _53);
			uint16_t _54 = *(BlockSrc + 256 * 13 + 1);
			*(BlockDst + 320 * 13 + 1) = Weight3_1(Weight1_3(_49, _50), Weight1_3(_53, _54));
			uint16_t _55 = *(BlockSrc + 256 * 13 + 2);
			*(BlockDst + 320 * 13 + 2) = Weight3_1(Weight1_1(_50, _51), Weight1_1(_54, _55));
			uint16_t _56 = *(BlockSrc + 256 * 13 + 3);
			*(BlockDst + 320 * 13 + 3) = Weight3_1(Weight3_1(_51, _52), Weight3_1(_55, _56));
			*(BlockDst + 320 * 13 + 4) = Weight3_1(_52, _56);

			// -- Row 15 --
			*(BlockDst + 320 * 14    ) = _53;
			*(BlockDst + 320 * 14 + 1) = Weight1_3(_53, _54);
			*(BlockDst + 320 * 14 + 2) = Weight1_1(_54, _55);
			*(BlockDst + 320 * 14 + 3) = Weight3_1(_55, _56);
			*(BlockDst + 320 * 14 + 4) = _56;

			// -- Row 16 --
			uint16_t _57 = *(BlockSrc + 256 * 14    );
			*(BlockDst + 320 * 15    ) = _57;
			uint16_t _58 = *(BlockSrc + 256 * 14 + 1);
			*(BlockDst + 320 * 15 + 1) = Weight1_3(_57, _58);
			uint16_t _59 = *(BlockSrc + 256 * 14 + 2);
			*(BlockDst + 320 * 15 + 2) = Weight1_1(_58, _59);
			uint16_t _60 = *(BlockSrc + 256 * 14 + 3);
			*(BlockDst + 320 * 15 + 3) = Weight3_1(_59, _60);
			*(BlockDst + 320 * 15 + 4) = _60;

			// -- Row 17 --
			uint16_t _61 = *(BlockSrc + 256 * 15    );
			*(BlockDst + 320 * 16    ) = _61;
			uint16_t _62 = *(BlockSrc + 256 * 15 + 1);
			*(BlockDst + 320 * 16 + 1) = Weight1_3(_61, _62);
			uint16_t _63 = *(BlockSrc + 256 * 15 + 2);
			*(BlockDst + 320 * 16 + 2) = Weight1_1(_62, _63);
			uint16_t _64 = *(BlockSrc + 256 * 15 + 3);
			*(BlockDst + 320 * 16 + 3) = Weight3_1(_63, _64);
			*(BlockDst + 320 * 16 + 4) = _64;

			BlockSrc += 4;
			BlockDst += 5;
		}
	}
}

void upscale_256x240_to_320x240_bilinearish(uint32_t* dst, uint32_t* src, int width)
{
	uint16_t* Src16 = (uint16_t*) src;
	uint16_t* Dst16 = (uint16_t*) dst;
	// There are 64 blocks of 4 pixels horizontally, and 239 of 1 vertically.
	// Each block of 4x1 becomes 5x1.
	uint32_t BlockX, BlockY;
	uint16_t* BlockSrc;
	uint16_t* BlockDst;
	for (BlockY = 0; BlockY < 239; BlockY++)
	{
		BlockSrc = Src16 + BlockY * 256 * 1;
		BlockDst = Dst16 + BlockY * 320 * 1;
		for (BlockX = 0; BlockX < 64; BlockX++)
		{
			/* Horizontally:
			 * Before(4):
			 * (a)(b)(c)(d)
			 * After(5):
			 * (a)(abbb)(bc)(cccd)(d)
			 */

			// -- Row 1 --
			uint16_t  _1 = *(BlockSrc               );
			*(BlockDst               ) = _1;
			uint16_t  _2 = *(BlockSrc            + 1);
			*(BlockDst            + 1) = Weight1_3( _1,  _2);
			uint16_t  _3 = *(BlockSrc            + 2);
			*(BlockDst            + 2) = Weight1_1( _2,  _3);
			uint16_t  _4 = *(BlockSrc            + 3);
			*(BlockDst            + 3) = Weight3_1( _3,  _4);
			*(BlockDst            + 4) = _4;

			BlockSrc += 4;
			BlockDst += 5;
		}
	}
}

/*
    Upscale 256x224 -> 320x240

    Horizontal upscale:
        320/256=1.25  --  do some horizontal interpolation
        8p -> 10p
        4dw -> 5dw

        coarse interpolation:
        [ab][cd][ef][gh] -> [ab][(bc)c][de][f(fg)][gh]

        fine interpolation
        [ab][cd][ef][gh] -> [a(0.25a+0.75b)][(0.5b+0.5c)(0.75c+0.25d)][de][(0.25e+0.75f)(0.5f+0.5g)][(0.75g+0.25h)h]

    Vertical upscale:
        Bresenham algo with simple interpolation

    Parameters:
        uint32_t *dst - pointer to 320x240x16bpp buffer
        uint32_t *src - pointer to 256x192x16bpp buffer
*/

void upscale_256x224_to_320x240(uint32_t *dst, uint32_t *src, int width)
{
    int midh = 240 / 2;
    int Eh = 0;
    int source = 0;
    int dh = 0;
    int y, x;

    for (y = 0; y < 240; y++)
    {
        source = dh * width / 2;

        for (x = 0; x < 320/10; x++)
        {
            register uint32_t ab, cd, ef, gh;

            __builtin_prefetch(dst + 4, 1);
            __builtin_prefetch(src + source + 4, 0);

            ab = src[source] & 0xF7DEF7DE;
            cd = src[source + 1] & 0xF7DEF7DE;
            ef = src[source + 2] & 0xF7DEF7DE;
            gh = src[source + 3] & 0xF7DEF7DE;

            if(Eh >= midh) {
                ab = AVERAGE(ab, src[source + width/2]) & 0xF7DEF7DE; // to prevent overflow
                cd = AVERAGE(cd, src[source + width/2 + 1]) & 0xF7DEF7DE; // to prevent overflow
                ef = AVERAGE(ef, src[source + width/2 + 2]) & 0xF7DEF7DE; // to prevent overflow
                gh = AVERAGE(gh, src[source + width/2 + 3]) & 0xF7DEF7DE; // to prevent overflow
            }

            *dst++ = ab;
            *dst++  = ((ab >> 17) + ((cd & 0xFFFF) >> 1)) + (cd << 16);
            *dst++  = (cd >> 16) + (ef << 16);
            *dst++  = (ef >> 16) + (((ef & 0xFFFF0000) >> 1) + ((gh & 0xFFFF) << 15));
            *dst++  = gh;

            source += 4;

        }
        Eh += 224; if(Eh >= 240) { Eh -= 240; dh++; }
    }
}

void upscale_256x240_to_320x240(uint32_t *dst, uint32_t *src, int width)
{
    int midh = 240 / 2;
    int Eh = 0;
    int source = 0;
    int dh = 0;
    int y, x;

    for (y = 0; y < 239; y++)
    {
        source = dh * width / 2;

        for (x = 0; x < 320/10; x++)
        {
            register uint32_t ab, cd, ef, gh;

            __builtin_prefetch(dst + 4, 1);
            __builtin_prefetch(src + source + 4, 0);

            ab = src[source] & 0xF7DEF7DE;
            cd = src[source + 1] & 0xF7DEF7DE;
            ef = src[source + 2] & 0xF7DEF7DE;
            gh = src[source + 3] & 0xF7DEF7DE;

            if(Eh >= midh) {
                ab = AVERAGE(ab, src[source + width/2]) & 0xF7DEF7DE; // to prevent overflow
                cd = AVERAGE(cd, src[source + width/2 + 1]) & 0xF7DEF7DE; // to prevent overflow
                ef = AVERAGE(ef, src[source + width/2 + 2]) & 0xF7DEF7DE; // to prevent overflow
                gh = AVERAGE(gh, src[source + width/2 + 3]) & 0xF7DEF7DE; // to prevent overflow
            }

            *dst++ = ab;
            *dst++  = ((ab >> 17) + ((cd & 0xFFFF) >> 1)) + (cd << 16);
            *dst++  = (cd >> 16) + (ef << 16);
            *dst++  = (ef >> 16) + (((ef & 0xFFFF0000) >> 1) + ((gh & 0xFFFF) << 15));
            *dst++  = gh;

            source += 4;

        }
        Eh += 239; if(Eh >= 239) { Eh -= 239; dh++; }
    }
}

/*
    Upscale 256x224 -> 384x240 (for 400x240)

    Horizontal interpolation
        384/256=1.5
        4p -> 6p
        2dw -> 3dw

        for each line: 4 pixels => 6 pixels (*1.5) (64 blocks)
        [ab][cd] => [a(ab)][bc][(cd)d]

    Vertical upscale:
        Bresenham algo with simple interpolation

    Parameters:
        uint32_t *dst - pointer to 400x240x16bpp buffer
        uint32_t *src - pointer to 256x192x16bpp buffer
        pitch correction is made
*/

void upscale_256x224_to_384x240_for_400x240(uint32_t *dst, uint32_t *src, int width)
{
    int midh = 240 / 2;
    int Eh = 0;
    int source = 0;
    int dh = 0;
    int y, x;

    dst += (400 - 384) / 4;

    for (y = 0; y < 240; y++)
    {
        source = dh * width / 2;

        for (x = 0; x < 384/6; x++)
        {
            register uint32_t ab, cd;

            __builtin_prefetch(dst + 4, 1);
            __builtin_prefetch(src + source + 4, 0);

            ab = src[source] & 0xF7DEF7DE;
            cd = src[source + 1] & 0xF7DEF7DE;

            if(Eh >= midh) {
                ab = AVERAGE(ab, src[source + width/2]) & 0xF7DEF7DE; // to prevent overflow
                cd = AVERAGE(cd, src[source + width/2 + 1]) & 0xF7DEF7DE; // to prevent overflow
            }

            *dst++ = (ab & 0xFFFF) + AVERAGEHI(ab);
            *dst++ = (ab >> 16) + ((cd & 0xFFFF) << 16);
            *dst++ = (cd & 0xFFFF0000) + AVERAGELO(cd);

            source += 2;

        }
        dst += (400 - 384) / 2;
        Eh += 224; if(Eh >= 240) { Eh -= 240; dh++; }
    }
}

/*
    Upscale 256x224 -> 384x272 (for 480x240)

    Horizontal interpolation
        384/256=1.5
        4p -> 6p
        2dw -> 3dw

        for each line: 4 pixels => 6 pixels (*1.5) (64 blocks)
        [ab][cd] => [a(ab)][bc][(cd)d]

    Vertical upscale:
        Bresenham algo with simple interpolation

    Parameters:
        uint32_t *dst - pointer to 480x272x16bpp buffer
        uint32_t *src - pointer to 256x192x16bpp buffer
        pitch correction is made
*/

void upscale_256x224_to_384x272_for_480x272(uint32_t *dst, uint32_t *src, int width)
{
    int midh = 272 / 2;
    int Eh = 0;
    int source = 0;
    int dh = 0;
    int y, x;

    dst += (480 - 384) / 4;

    for (y = 0; y < 272; y++)
    {
        source = dh * width / 2;

        for (x = 0; x < 384/6; x++)
        {
            register uint32_t ab, cd;

            __builtin_prefetch(dst + 4, 1);
            __builtin_prefetch(src + source + 4, 0);

            ab = src[source] & 0xF7DEF7DE;
            cd = src[source + 1] & 0xF7DEF7DE;

            if(Eh >= midh) {
                ab = AVERAGE(ab, src[source + width/2]) & 0xF7DEF7DE; // to prevent overflow
                cd = AVERAGE(cd, src[source + width/2 + 1]) & 0xF7DEF7DE; // to prevent overflow
            }

            *dst++ = (ab & 0xFFFF) + AVERAGEHI(ab);
            *dst++ = (ab >> 16) + ((cd & 0xFFFF) << 16);
            *dst++ = (cd & 0xFFFF0000) + AVERAGELO(cd);

            source += 2;

        }
        dst += (480 - 384) / 2;
        Eh += 224; if(Eh >= 272) { Eh -= 272; dh++; }
    }
}
