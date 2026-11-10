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
int gfx_ega_caps(), gfx_ega_clear(), gfx_ega_byte(), gfx_ega_font();
int gfx_hgc_byte(), gfx_hgc_detect(), gfx_hgc_graphics(), gfx_hgc_text();
int gfx_hgc_clear();
int gfx_vga_clear(), gfx_vga_byte(), gfx_vga_dac();
int gfx_bios_palette_entry(), gfx_bios_dac();

#define BACKEND_BIOS    1
#define BACKEND_KERNEL  2

static int backend = 0;
static int hgc_direct = 0;      /* Hercules driven by the program itself */
static int current_mode = MODE_TEXT;
static int palette_reg = 0;

/* Native XIOS capability bits returned by the selector-30 probe. */
#define CAP_EGA_350     1
#define CAP_EGA_200     2
#define CAP_VGA         4
#define CAP_HERCULES    16

static int is_cga(mode)
    int mode;
{
    return mode == MODE_CGA_320X200 || mode == MODE_CGA_640X200;
}

static int is_hgc(mode)
    int mode;
{
    return mode == MODE_HERCULES;
}

/* Enhanced modes reached through the patched XIOS. 0Fh stays BIOS-only. */
static int native_cap(mode)
    int mode;
{
    switch (mode) {
    case MODE_EGA_640X350:
        return CAP_EGA_350;
    case MODE_EGA_320X200:
    case MODE_EGA_640X200:
        return CAP_EGA_200;
    case MODE_VGA_640X480_MONO:
    case MODE_VGA_640X480:
    case MODE_VGA_320X200:
        return CAP_VGA;
    case MODE_HERCULES:
        return CAP_HERCULES;
    }
    return 0;
}

static int mode_width(mode)
    int mode;
{
    if (mode == MODE_HERCULES)
        return 720;
    return mode == MODE_CGA_320X200 || mode == MODE_EGA_320X200 ||
           mode == MODE_VGA_320X200 ? 320 : 640;
}

static int mode_height(mode)
    int mode;
{
    if (mode == MODE_HERCULES)
        return 348;
    if (mode == MODE_EGA_640X350 || mode == MODE_EGA_640X350_MONO)
        return 350;
    if (mode == MODE_VGA_640X480 || mode == MODE_VGA_640X480_MONO)
        return 480;
    return 200;
}

/* ROM graphics font height: 8x14 at 350 lines, 8x16 at 480, else 8x8. */
static int font_height(mode)
    int mode;
{
    int height = mode_height(mode);
    return height == 350 ? 14 : height == 480 ? 16 : 8;
}

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
 * Use this extension on the Concurrent 3.1 and 4.1 XIOS.
 */
static int enhanced_xios_version()
{
    int version = osver();
    return (version == 0x31 || version == 0x41) &&
           (ostype() & OSTYPE_MULTIUSER);
}

static void kernel_palette(value)
    int value;
{
    int i;
    static unsigned char request[8] = { 30, 0, 0, 0, 0, 2, 0, 0 };

    gfx_cga_palette(value);
    if (!enhanced_xios_version())
        return;
    request[4] = value;
    cputc((char)27);
    cputc((char)'!');
    for (i = 0; i < 8; i++)
        cputc((char)request[i]);
}

/* Hercules text restore. ESC a cannot do it: the Hercules XIOS accepts only
 * native text modes 09h and 01h, and no ESC a parameter maps to either. Use
 * the function-30 back door with CH=0 (set mode). CL must be 01h, not 09h:
 * the packet goes through BDOS console output, which expands a TAB (09h).
 */
static void kernel_hgc_text()
{
    int i;
    static unsigned char request[8] = { 30, 0, 0, 0, 1, 0, 0, 0 };

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
    int cap = native_cap(mode);
    int native = selected == BACKEND_KERNEL && cap && enhanced_xios_version();
    /* CP/M-86 has no Hercules mode in its BIOS: program the card directly. */
    int direct = is_hgc(mode) && selected == BACKEND_BIOS && gfx_hgc_detect();

    if (!selected)
        return -1;
    if (native && !(gfx_ega_caps() & cap))
        return -1;
    if (mode != MODE_TEXT && !is_cga(mode) && !native && !direct &&
        !(selected == BACKEND_BIOS && mode >= MODE_EGA_320X200 &&
          mode <= MODE_VGA_320X200))
        return -1;
    if (mode != MODE_TEXT)
        statline(STATLINE_OFF);
    if (selected == BACKEND_BIOS) {
        if (direct)
            gfx_hgc_graphics();
        else if (hgc_direct)
            gfx_hgc_text();
        if (!direct && !(hgc_direct && mode == MODE_TEXT))
            gfx_bios_mode(mode);
    } else {
        if (mode == MODE_TEXT && is_hgc(current_mode)) {
            kernel_hgc_text();
        } else {
            cputc((char)27);
            cputc((char)'a');
            cputc((char)mode);
        }
        if (mode == MODE_VGA_320X200) {
            gfx_vga_clear();
        } else if (is_hgc(mode)) {
            /* The kernel redraws the status line into B000:0F00, inside the
             * graphics page. Turn it off again, then clear the page.
             */
            statline(STATLINE_OFF);
            gfx_hgc_clear();
        } else if (native) {
            /* Words per plane: 40 or 80 bytes per line. */
            gfx_ega_clear((unsigned)(mode_width(mode) >> 3) *
                          mode_height(mode) / 2);
        } else if (mode != MODE_TEXT) {
            gfx_cga_clear();
            palette_reg = mode == MODE_CGA_320X200 ? 0x30 : 0x0f;
            kernel_palette(palette_reg);
        }
    }
    if (mode == MODE_TEXT && current_mode != MODE_TEXT)
        statline(STATLINE_ON);
    hgc_direct = direct;
    backend = direct ? BACKEND_KERNEL : selected;
    current_mode = mode;
    return 0;
}

void gfx_palette(palette)
    int palette;
{
    int operation;

    if (backend == BACKEND_BIOS) {
        gfx_bios_palette(palette);
    } else if (backend == BACKEND_KERNEL && is_cga(current_mode)) {
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
 * Planar modes 0Dh..12h only; mode 13h uses gfx_dac_entry.
 */
int gfx_palette_entry(index, color)
    int index, color;
{
    int i;
    unsigned char request[8];

    if (index < 0 || index > 15 || color < 0 || color > 63)
        return -1;
    if (current_mode < MODE_EGA_320X200 || current_mode > MODE_VGA_640X480)
        return -1;
    if (backend == BACKEND_BIOS) {
        gfx_bios_palette_entry(index, color);
        return 0;
    }
    if (backend != BACKEND_KERNEL || current_mode == MODE_EGA_640X350_MONO)
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

/* VGA DAC entry 0..255 with 6-bit red/green/blue (0..63). VGA modes
 * 11h..13h only. XIOS: direct DAC ports while this program owns graphics.
 */
int gfx_dac_entry(index, red, green, blue)
    int index, red, green, blue;
{
    if (index < 0 || index > 255 || red < 0 || red > 63 ||
        green < 0 || green > 63 || blue < 0 || blue > 63)
        return -1;
    if (current_mode < MODE_VGA_640X480_MONO ||
        current_mode > MODE_VGA_320X200)
        return -1;
    if (backend == BACKEND_BIOS)
        gfx_bios_dac(index, red, green, blue);
    else if (backend == BACKEND_KERNEL)
        gfx_vga_dac(index, red, green, blue);
    else
        return -1;
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
    if (x >= mode_width(current_mode) || y >= mode_height(current_mode))
        return;
    if (current_mode == MODE_VGA_320X200) {
        gfx_vga_byte((unsigned)y * 320 + x, color);
        return;
    }
    if (is_hgc(current_mode)) {
        /* Four interleaved 8 KB banks, 90 bytes per line. */
        shift = 7 - (x & 7);
        gfx_hgc_byte((y & 3) * 8192 + (y >> 2) * 90 + (x >> 3),
                     1 << shift, (color & 1) << shift, color & 0x80);
        return;
    }
    if (!is_cga(current_mode)) {
        if (current_mode == MODE_VGA_640X480_MONO)
            color &= 0x81;
        gfx_ega_byte((unsigned)y * (mode_width(current_mode) >> 3) +
                     (x >> 3), 128 >> (x & 7), color);
        return;
    }
    if (current_mode == MODE_CGA_320X200) {
        shift = (3 - (x & 3)) * 2;
        mask = 3 << shift;
        bits = (color & 3) << shift;
        offset = x >> 2;
    } else {
        shift = 7 - (x & 7);
        mask = 1 << shift;
        bits = (color & 1) << shift;
        offset = x >> 3;
    }
    offset += (y >> 1) * 80 + ((y & 1) ? 8192 : 0);
    gfx_cga_byte(offset, mask, bits, color & 0x80);
}

/* Same cell coordinates as BIOS text, with an opaque black background.
 * The kernel path supports the IBM PC ROM's ASCII font (0..127): 8x8 in
 * CGA, 0Dh, 0Eh and 13h; 8x14 in 10h; 8x16 and 30 rows in 11h/12h.
 */
void gfx_text(col, row, str, color)
    int col, row;
    char *str;
    int color;
{
    int ch, y, x, bits, columns, rows, height, stride;
    unsigned offset;

    if (backend == BACKEND_BIOS) {
        gfx_bios_text(col, row, str, color);
        return;
    }
    if (backend != BACKEND_KERNEL || current_mode == MODE_TEXT)
        return;
    height = font_height(current_mode);
    columns = mode_width(current_mode) >> 3;
    rows = mode_height(current_mode) / height;
    stride = columns;
    if (!color)
        color = 15;
    if (current_mode != MODE_VGA_320X200)
        color &= (current_mode == MODE_VGA_640X480_MONO ||
                  is_hgc(current_mode)) ? 1 : 15;
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
        if (row >= rows)
            break;
        if (ch >= 128)
            ch = '?';
        for (y = 0; y < height; y++) {
            if (is_cga(current_mode) || is_hgc(current_mode)) {
                bits = gfx_cga_font(ch, y);
            } else {
                bits = gfx_ega_font(ch, y, height);
            }
            if (is_hgc(current_mode)) {
                /* Byte-aligned cell: 90 cells of 8 pixels, opaque background. */
                offset = ((row * 8 + y) & 3) * 8192 +
                         ((row * 8 + y) >> 2) * 90 + col;
                gfx_hgc_byte(offset, 255, color ? bits : 0, 0);
            } else if (is_cga(current_mode) || current_mode == MODE_VGA_320X200) {
                for (x = 0; x < 8; x++)
                    gfx_pixel(col * 8 + x, row * height + y,
                              (bits & (128 >> x)) ? color : 0);
            } else {
                /* Planar cells are byte aligned: foreground then background. */
                offset = (unsigned)(row * height + y) * stride + col;
                gfx_ega_byte(offset, bits, color);
                gfx_ega_byte(offset, ~bits & 255, 0);
            }
        }
        col++;
    }
}
