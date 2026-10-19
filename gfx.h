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

/* gfx_mode returns 0 when issued, -1 for an unsupported OS or mode.
 * BIOS: BDOS 22h or >=60h. XIOS: Concurrent 31h..5Fh, DOS Plus 41h.
 * XIOS modes 3/4/6 require CGA, except patched ccpm86-kernel also supports
 * EGA/VGA. Native mode 10h requires its capability query to succeed.
 * XIOS text: 8x8 PC ROM in CGA, BIOS-selected 8x14 ROM in mode 10h.
 * gfx_palette_entry uses raw 6-bit EGA colors (0..63), indices 0..15.
 * See GRAPHICS.md for adapter and OS restrictions.
 */
#ifndef __STDC__
int gfx_mode();
void gfx_palette();
int gfx_palette_entry();
void gfx_pixel();
void gfx_text();
#else
int gfx_mode(int mode);
void gfx_palette(int palette);
int gfx_palette_entry(int index, int color);
void gfx_pixel(int x, int y, int palette);
void gfx_text(int col, int row, char *str, int color);
#endif

#endif
