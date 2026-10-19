/*
 * Licensed under the MIT license. See LICENSE in the project root.
 * IBM PC graphics: BIOS on BDOS 22h or >= 60h; console escape modes on
 * Concurrent BDOS 31h..5Fh and non-Concurrent BDOS 41h (DOS Plus).
 */
#include <stdio.h>
#include <conio.h>
#include <util.h>
#include <gfx.h>

int gfx_bios_mode(), gfx_bios_palette(), gfx_bios_pixel(), gfx_bios_text();
int gfx_cga_clear(), gfx_cga_byte(), gfx_cga_palette(), gfx_cga_font();

#define BACKEND_BIOS    1
#define BACKEND_KERNEL  2

static int backend = 0;
static int current_mode = MODE_TEXT;
static int palette_reg = 0;

static int select_backend()
{
    int version = osver();
    int concurrent = ostype() & OSTYPE_MULTIUSER;

    if (version == 0x22 || version >= 0x60)
        return BACKEND_BIOS;
    if ((version >= 0x31 && concurrent) ||
        (version == 0x41 && !concurrent))
        return BACKEND_KERNEL;
    return 0;
}

/* Preserve direct CGA palette writes for existing kernels. The function-30
 * escape back door is accepted by the enhanced XIOS; older XIOS versions
 * consume the complete packet and reject the call without printing it.
 */
static void kernel_palette(value)
    int value;
{
    int i;
    static unsigned char request[8] = { 30, 0, 0, 0, 0, 2, 0, 0 };

    gfx_cga_palette(value);
    if (osver() != 0x31)
        return;
    request[4] = value;
    cputc((char)27);
    cputc((char)'!');
    for (i = 0; i < 8; i++)
        cputc((char)request[i]);
}

/* No display changes on unsupported requests. Escape output itself has
 * no acknowledgement: success means the mode request was issued.
 */
int gfx_mode(mode)
    int mode;
{
    int selected = select_backend();

    if (!selected)
        return -1;
    if (mode != MODE_TEXT && mode != MODE_CGA_320X200 &&
        mode != MODE_CGA_640X200 &&
        !(selected == BACKEND_BIOS && mode >= MODE_EGA_320X200 &&
          mode <= MODE_EGA_640X350))
        return -1;
    if (mode != MODE_TEXT)
        statline(STATLINE_OFF);
    if (selected == BACKEND_BIOS) {
        gfx_bios_mode(mode);
    } else {
        cputc((char)27);
        cputc((char)'a');
        cputc((char)mode);
        if (mode != MODE_TEXT) {
            gfx_cga_clear();
            palette_reg = mode == MODE_CGA_320X200 ? 0x30 : 0x0f;
            kernel_palette(palette_reg);
        }
    }
    if (mode == MODE_TEXT && current_mode != MODE_TEXT)
        statline(STATLINE_ON);
    backend = selected;
    current_mode = mode;
    return 0;
}

void gfx_palette(palette)
    int palette;
{
    int operation;

    if (backend == BACKEND_BIOS) {
        gfx_bios_palette(palette);
    } else if (backend == BACKEND_KERNEL && current_mode != MODE_TEXT) {
        operation = (palette >> 8) & 255;
        if (operation == 1)
            palette_reg = (palette_reg & ~0x20) | ((palette & 1) << 5);
        else if (operation == 0)
            palette_reg = (palette_reg & ~0x1f) | (palette & 0x1f);
        else
            return;
        kernel_palette(palette_reg);
    }
}

void gfx_pixel(x, y, color)
    int x, y, color;
{
    int shift, mask, bits, offset;

    if (backend == BACKEND_BIOS) {
        gfx_bios_pixel(x, y, color);
        return;
    }
    if (backend != BACKEND_KERNEL || current_mode == MODE_TEXT ||
        x < 0 || y < 0 || y >= 200)
        return;
    if (current_mode == MODE_CGA_320X200) {
        if (x >= 320)
            return;
        shift = (3 - (x & 3)) * 2;
        mask = 3 << shift;
        bits = (color & 3) << shift;
        offset = x >> 2;
    } else {
        if (x >= 640)
            return;
        shift = 7 - (x & 7);
        mask = 1 << shift;
        bits = (color & 1) << shift;
        offset = x >> 3;
    }
    offset += (y >> 1) * 80 + ((y & 1) ? 8192 : 0);
    gfx_cga_byte(offset, mask, bits, color & 0x80);
}

/* Same cell coordinates as BIOS text, with an opaque black background.
 * The kernel path supports the IBM PC ROM's ASCII font (0..127).
 */
void gfx_text(col, row, str, color)
    int col, row;
    char *str;
    int color;
{
    int ch, y, x, bits, columns;

    if (backend == BACKEND_BIOS) {
        gfx_bios_text(col, row, str, color);
        return;
    }
    if (backend != BACKEND_KERNEL || current_mode == MODE_TEXT)
        return;
    columns = current_mode == MODE_CGA_320X200 ? 40 : 80;
    if (!color)
        color = 15;
    while (*str) {
        ch = *str++ & 255;
        if (ch == '\r') {
            col = 0;
            continue;
        }
        if (ch == '\n') {
            row++;
            continue;
        }
        if (col >= columns) {
            col = 0;
            row++;
        }
        if (row >= 25)
            break;
        if (ch >= 128)
            ch = '?';
        for (y = 0; y < 8; y++) {
            bits = gfx_cga_font(ch, y);
            for (x = 0; x < 8; x++)
                gfx_pixel(col * 8 + x, row * 8 + y,
                          (bits & (128 >> x)) ? color & 15 : 0);
        }
        col++;
    }
}
