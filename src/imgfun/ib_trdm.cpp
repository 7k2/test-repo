/*
 * Seven Kingdoms 2: The Fryhtan War
 *
 * Copyright 1999 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *Filename    : ib_trdm.cpp
 *Description : Blt a bitmap to the display surface buffer
 *              with decompression, transparency handling and colormaping
 *              and horizontal mirroring
 *
 * converted to C++
 */

#include <imgfun.h>
#include <colcode.h>

//----------- BEGIN OF FUNCTION IMGbltTransRemapDecompressHMirror ------
//
// Put a compressed bitmap on image buffer.
// It does handle color key transparency and remaping.
//
// Syntax : IMGbltTransRemapDecompressHMirror( imageBuf, pitch, x, y, bitmapBuf,
//                 colorTable)
//
// short *imageBuf  - the pointer to the display surface buffer
// int  pitch       - pitch of the display surface buffer
// int  x,y         - where to put the image on the surface buffer
// char *bitmapPtr  - the pointer to the bitmap buffer
// char *colorTable - color remaping table
//
// two counters are maintained, EDX and ECX for counting no. of rows
// and no. of columns remaining to draw
// if the counter reach zero, exit the loop
//
// ESI initally points to the start of bitmap data
// EDI initally points to the top right hand cornder of the destination
//     in video memory
//
// compressed data is loaded from ESI, into AL
// If AL is non-transparent, blit the point to video memory.
// If AL is transparent, seek EDI forward. If the right side of the
// destination is passed,
//   1. seek EDI to the right side of the next line
//   2. if run-length is still very long, seek one more line
//   3. residue (of run-length) subtracts EDI, ECX will count from a number
//      lower than the width of bitmap
//
//-------------------------------------------------
//
// Format of the bitmap data :
//
// <short>  width
// <short>  height
// <char..> bitmap image
//
//-------------------------------------------------

void IMGbltTransRemapDecompressHMirror(short *imageBuf, int pitch, int desX, int desY, char *bitmapBuf, short *colorTable)
{
	int destline = desY * (pitch / 2) + desX;
	int width = ((unsigned char*)bitmapBuf)[0] + (((unsigned char*)bitmapBuf)[1]<<8);
	int height = ((unsigned char*)bitmapBuf)[2] + (((unsigned char*)bitmapBuf)[3]<<8);
	int esi = 4;		// 4 bytes of bitmap header (16bit width, 16bit height)
	int pixelsToSkip = 0;
	int al;

	for ( int j=0; j<height; ++j,destline+=pitch/2 )
	{
		for ( int i=width-1; i>=0; --i )	// NOTE: this is descending (mirrored)
		{
			if (pixelsToSkip != 0)
			{
				if (pixelsToSkip > i)
				{
					// skip to next line
					pixelsToSkip -= i+1;
					break;
				}
				i -= pixelsToSkip;
				pixelsToSkip = 0;
			}
			al = ((unsigned char*)bitmapBuf)[ esi++ ];		// load source byte
			if (al < MIN_EFFECT_CODE)
			{
				imageBuf[ destline + i ] = colorTable[ al ];	// normal pixel
			}
			else if (al < MANY_TRANSPARENT_CODE)
			{
				imageBuf[ destline + i ] = effect_proc(al, imageBuf[ destline + i ]);
			}
			else if (al == MANY_TRANSPARENT_CODE)
			{
				pixelsToSkip = ((unsigned char*)bitmapBuf)[ esi++ ] -1;		// skip many pixels
			}
			else
			{
				pixelsToSkip = 256 - al -1;					// skip (neg al) pixels
			}
		}
	}
}

//----------- END OF FUNCTION IMGbltTransRemapDecompressHMirror ----------
