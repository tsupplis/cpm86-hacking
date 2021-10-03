#ifndef _GFX_H_
#define _GFX_H_

/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#define CGA_PALETTE_CMW 0x101
#define CGA_PALETTE_GRY 0x100

#define MODE_CGA_320X200 4
#define MODE_CGA_640X200 6
#define MODE_TEXT        3

#ifndef __STDC__
gfx_mode();
gfx_palette();
gfx_pixel();
#else
void gfx_mode(int mode);
void gfx_palette(int palette);
void gfx_pixel(int x, int y, int palette);
#endif

#endif
