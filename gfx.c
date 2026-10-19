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
int gfx_ega_available(), gfx_ega_clear(), gfx_ega_byte(), gfx_ega_font();
int gfx_bios_palette_entry();

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
 * Send the extension only to 3.1; keep later XIOS implementations unchanged.
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
    int native_ega = selected == BACKEND_KERNEL &&
        mode == MODE_EGA_640X350 && osver() == 0x31;

    if (!selected)
        return -1;
    if (native_ega && !gfx_ega_available())
        return -1;
    if (mode != MODE_TEXT && mode != MODE_CGA_320X200 &&
        mode != MODE_CGA_640X200 && !native_ega &&
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
        if (native_ega) {
            gfx_ega_clear();
        } else if (mode != MODE_TEXT) {
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
    } else if (backend == BACKEND_KERNEL && current_mode != MODE_TEXT &&
               current_mode != MODE_EGA_640X350) {
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

/* Raw six-bit EGA color: bits 0..2 are primary B/G/R, bits 3..5
 * are their secondary components. Kept separate from CGA palette selection.
 */
int gfx_palette_entry(index, color)
    int index, color;
{
    int i;
    unsigned char request[8];

    if (index < 0 || index > 15 || color < 0 || color > 63)
        return -1;
    if (backend == BACKEND_BIOS && current_mode >= MODE_EGA_320X200 &&
        current_mode <= MODE_EGA_640X350) {
        gfx_bios_palette_entry(index, color);
        return 0;
    }
    if (backend != BACKEND_KERNEL || current_mode != MODE_EGA_640X350)
        return -1;
    for (i = 0; i < 8; i++)
        request[i] = 0;
    request[0] = 30;
    request[4] = index;
    request[5] = 3;
    request[7] = color;
    cputc((char)27);
    cputc((char)'!');
    for (i = 0; i < 8; i++)
        cputc((char)request[i]);
    return 0;
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
        x < 0 || y < 0)
        return;
    if (current_mode == MODE_EGA_640X350) {
        if (x < 640 && y < 350)
            gfx_ega_byte(y * 80 + (x >> 3), 128 >> (x & 7), color);
        return;
    }
    if (y >= 200)
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
    int ch, y, x, bits, columns, height;

    if (backend == BACKEND_BIOS) {
        gfx_bios_text(col, row, str, color);
        return;
    }
    if (backend != BACKEND_KERNEL || current_mode == MODE_TEXT)
        return;
    height = current_mode == MODE_EGA_640X350 ? 14 : 8;
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
        for (y = 0; y < height; y++) {
            bits = height == 14 ? gfx_ega_font(ch, y) : gfx_cga_font(ch, y);
            for (x = 0; x < 8; x++)
                gfx_pixel(col * 8 + x, row * height + y,
                          (bits & (128 >> x)) ? color & 15 : 0);
        }
        col++;
    }
}
