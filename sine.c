/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/
#include <stdio.h>
#include <math.h>
#include <conio.h>
#include <util.h>
#include <gfx.h>

#define SCREEN_WIDTH   640
#define SCREEN_HEIGHT  350

#define ORIGIN_X       320
#define ORIGIN_Y       175
#define HALF_WIDTH     240
#define PLOT_WIDTH     480
#define AMPLITUDE      120

#define COLOR_BG       0
#define COLOR_AXIS     10  /* Light Green */
#define COLOR_SINE     12  /* Light Red */
#define COLOR_ACCENT   14  /* Yellow */
#define COLOR_TEXT     15  /* Bright White */

#define PI             3.14159265358979323846

#ifndef __STDC__
void draw_line(x0, y0, x1, y1, color)
    int x0;
    int y0;
    int x1;
    int y1;
    int color;
#else
void draw_line(int x0, int y0, int x1, int y1, int color)
#endif
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;
    int err, e2;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    err = dx - dy;

    while (1) {
        if (x0 >= 0 && x0 < SCREEN_WIDTH && y0 >= 0 && y0 < SCREEN_HEIGHT)
            gfx_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = err << 1;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

#ifndef __STDC__
void draw_axes()
#else
void draw_axes(void)
#endif
{
    int i;

    /* X axis: from x = 30 to x = 610 at y = ORIGIN_Y */
    draw_line(30, ORIGIN_Y, 610, ORIGIN_Y, COLOR_AXIS);
    /* X axis arrowhead */
    draw_line(610, ORIGIN_Y, 600, ORIGIN_Y - 5, COLOR_AXIS);
    draw_line(610, ORIGIN_Y, 600, ORIGIN_Y + 5, COLOR_AXIS);

    /* Y axis: from y = 325 down to y = 25 at x = ORIGIN_X (center) */
    draw_line(ORIGIN_X, 325, ORIGIN_X, 25, COLOR_AXIS);
    /* Y axis arrowhead */
    draw_line(ORIGIN_X, 25, ORIGIN_X - 6, 35, COLOR_AXIS);
    draw_line(ORIGIN_X, 25, ORIGIN_X + 6, 35, COLOR_AXIS);

    /* X axis tick marks at -2pi, -3pi/2, -pi, -pi/2, pi/2, pi, 3pi/2, 2pi */
    for (i = -4; i <= 4; i++) {
        if (i == 0)
            continue;
        {
            int tx = ORIGIN_X + i * (HALF_WIDTH / 4);
            int tick_h = (i % 2 == 0) ? 6 : 3; /* Major tick at multiples of pi */
            draw_line(tx, ORIGIN_Y - tick_h, tx, ORIGIN_Y + tick_h, COLOR_ACCENT);
        }
    }

    /* Y axis tick marks at +1.0, +0.5, -0.5, -1.0 */
    draw_line(ORIGIN_X - 6, ORIGIN_Y - AMPLITUDE, ORIGIN_X + 6, ORIGIN_Y - AMPLITUDE, COLOR_ACCENT);
    draw_line(ORIGIN_X - 4, ORIGIN_Y - (AMPLITUDE / 2), ORIGIN_X + 4, ORIGIN_Y - (AMPLITUDE / 2), COLOR_ACCENT);
    draw_line(ORIGIN_X - 4, ORIGIN_Y + (AMPLITUDE / 2), ORIGIN_X + 4, ORIGIN_Y + (AMPLITUDE / 2), COLOR_ACCENT);
    draw_line(ORIGIN_X - 6, ORIGIN_Y + AMPLITUDE, ORIGIN_X + 6, ORIGIN_Y + AMPLITUDE, COLOR_ACCENT);

    /* Text labels in bright white */
    gfx_text(42, 2, "y", COLOR_TEXT);
    gfx_text(77, 12, "x", COLOR_TEXT);
    gfx_text(58, 3, "f(x)=sin(x)", COLOR_TEXT);
}

#ifndef __STDC__
void sine_demo()
#else
void sine_demo(void)
#endif
{
    int i, x, y;
    int prev_x = -1;
    int prev_y = -1;
    double rad, s;

    draw_axes();

    /* Plot sine curve y = sin(x) for x in [-2*PI, +2*PI] across 640x350 EGA */
    for (i = 0; i <= PLOT_WIDTH; i++) {
        x = (ORIGIN_X - HALF_WIDTH) + i;
        rad = ((double)(i - HALF_WIDTH) * (2.0 * PI)) / (double)HALF_WIDTH;
        s = sin(rad);
        y = ORIGIN_Y - (int)(s * (double)AMPLITUDE);

        if (prev_x >= 0) {
            draw_line(prev_x, prev_y, x, y, COLOR_SINE);
        } else {
            gfx_pixel(x, y, COLOR_SINE);
        }
        prev_x = x;
        prev_y = y;
    }

    /* Wait for user keypress */
    while (!kbhit())
        ;
    getch();
}

#ifndef __STDC__
int main(argc, argv)
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
    freopen("con:", "r", stdin);
    freopen("con:", "w", stdout);
    if (gfx_mode(MODE_EGA_640X350) < 0) {
        fprintf(stderr, "Graphics mode unsupported on this system.\n");
        return 1;
    }

    sine_demo();

    gfx_mode(MODE_TEXT);
    clrscr();
    return 0;
}
