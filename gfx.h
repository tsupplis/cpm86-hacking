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
#define MODE_VGA_640X480_MONO   17      /* 0x11: 640x480, 2 colors */
#define MODE_VGA_640X480        18      /* 0x12: 640x480, 16 colors */
#define MODE_VGA_320X200        19      /* 0x13: 320x200, 256 colors */

/* gfx_mode returns 0 when issued, -1 for an unsupported OS or mode.
 * BIOS: BDOS 22h or >=60h, modes 3/4/6 and 0Dh..13h.
 * XIOS: Concurrent 31h..5Fh, DOS Plus 41h. Modes 3/4/6 require CGA, except
 * patched ccpm86-kernel also supports EGA/VGA. Native modes 0Dh/0Eh/10h and
 * VGA 11h/12h/13h require the matching capability bit; 0Fh is BIOS-only.
 * XIOS text: 8x8 PC ROM in CGA, BIOS-selected ROM font otherwise
 * (8x8 in 0Dh/0Eh/13h, 8x14 in 10h, 8x16 with 30 rows in 11h/12h).
 * gfx_palette_entry uses raw 6-bit EGA colors (0..63), indices 0..15, in
 * planar modes 0Dh..12h. gfx_dac_entry sets VGA DAC entries 0..255 with
 * 6-bit RGB in modes 11h..13h. In mode 13h, gfx_pixel stores the full
 * colour byte (no XOR); elsewhere colour bit 7 requests XOR.
 * See GRAPHICS.md for adapter and OS restrictions.
 */
#ifndef __STDC__
int gfx_mode();
void gfx_palette();
int gfx_palette_entry();
int gfx_dac_entry();
void gfx_pixel();
void gfx_text();
#else
int gfx_mode(int mode);
void gfx_palette(int palette);
int gfx_palette_entry(int index, int color);
int gfx_dac_entry(int index, int red, int green, int blue);
void gfx_pixel(int x, int y, int palette);
void gfx_text(int col, int row, char *str, int color);
#endif

#endif
