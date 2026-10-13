#ifndef _GFX_H_
#define _GFX_H_

/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#define CGA_PALETTE_CMW         0x101
#define CGA_PALETTE_GRY         0x100

#define MODE_TEXT               3
#define MODE_CGA_320X200        4
#define MODE_CGA_640X200        6
#define MODE_EGA_320X200        13      /* 0x0D: 320x200, 16 colors */
#define MODE_EGA_640X200        14      /* 0x0E: 640x200, 16 colors */
#define MODE_EGA_640X350_MONO   15      /* 0x0F: 640x350, monochrome */
#define MODE_EGA_640X350        16      /* 0x10: 640x350, 16 colors */

#ifndef __STDC__
gfx_mode();
gfx_palette();
gfx_pixel();
gfx_text();
#else
void gfx_mode(int mode);
void gfx_palette(int palette);
void gfx_pixel(int x, int y, int palette);
void gfx_text(int col, int row, char *str, int color);
#endif

#endif
