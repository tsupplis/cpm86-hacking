/*
  Licensed under the MIT license. See LICENSE file in the project root for details.

  Mystic rose for Hercules (720x348, monochrome). Needs the CDOS 4.1 kernel
  Hercules extension: ESC a 7Fh, capability bit 4.
*/
#include <stdio.h>
#include <math.h>
#include <conio.h>
#include <util.h>
#include <gfx.h>

#define SCREEN_WIDTH   720
#define SCREEN_HEIGHT  348

#define ORIGIN_X       360
#define ORIGIN_Y       182
#define RADIUS         160
#define POINTS         10
#define CIRCLE_STEPS   120

#define COLOR_ON       1
#define COLOR_TEXT     1

#define PI             3.14159265358979323846

/* One colour per span is impossible on one bit: use a line pattern instead.
 * Bit (step & 7) of the pattern selects a lit pixel. Neighbours (1) out to
 * diameters (POINTS / 2).
 */
static int span_patterns[] = { 0, 0xFF, 0xEE, 0xCC, 0xAA, 0x88 };

#define PATTERN_CIRCLE 0xFF

static int px[POINTS];
static int py[POINTS];

#ifndef __STDC__
void draw_line(x0, y0, x1, y1, pattern)
    int x0;
    int y0;
    int x1;
    int y1;
    int pattern;
#else
void draw_line(int x0, int y0, int x1, int y1, int pattern)
#endif
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;
    int err, e2;
    int step = 0;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    err = dx - dy;

    while (1) {
        if ((pattern >> (step & 7)) & 1 &&
            x0 >= 0 && x0 < SCREEN_WIDTH && y0 >= 0 && y0 < SCREEN_HEIGHT)
            gfx_pixel(x0, y0, COLOR_ON);
        step++;
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
void draw_circle()
#else
void draw_circle(void)
#endif
{
    int i, x, y;
    int prev_x = ORIGIN_X + RADIUS;
    int prev_y = ORIGIN_Y;
    double rad;

    for (i = 1; i <= CIRCLE_STEPS; i++) {
        rad = ((double)i * 2.0 * PI) / (double)CIRCLE_STEPS;
        x = ORIGIN_X + (int)(cos(rad) * (double)RADIUS);
        y = ORIGIN_Y + (int)(sin(rad) * (double)RADIUS);
        draw_line(prev_x, prev_y, x, y, PATTERN_CIRCLE);
        prev_x = x;
        prev_y = y;
    }
}

#ifndef __STDC__
void draw_point(x, y)
    int x;
    int y;
#else
void draw_point(int x, int y)
#endif
{
    int dx, dy;

    for (dy = -2; dy <= 2; dy++)
        for (dx = -2; dx <= 2; dx++)
            if (dx * dx + dy * dy <= 5)
                gfx_pixel(x + dx, y + dy, COLOR_ON);
}

#ifndef __STDC__
void rose_demo()
#else
void rose_demo(void)
#endif
{
    int i, j, span;
    double rad;

    gfx_text(34, 1, "Mystic rose: 10 points", COLOR_TEXT);
    draw_circle();

    /* Vertices evenly spaced on the circle, the first one at the top. */
    for (i = 0; i < POINTS; i++) {
        rad = ((double)i * 2.0 * PI) / (double)POINTS - PI / 2.0;
        px[i] = ORIGIN_X + (int)(cos(rad) * (double)RADIUS);
        py[i] = ORIGIN_Y + (int)(sin(rad) * (double)RADIUS);
    }

    /* Join every pair of vertices: 10 * 9 / 2 = 45 chords. */
    for (span = 1; span <= POINTS / 2; span++) {
        for (i = 0; i < POINTS; i++) {
            j = (i + span) % POINTS;
            /* Diameters would otherwise be drawn twice. */
            if (span * 2 == POINTS && j < i)
                continue;
            draw_line(px[i], py[i], px[j], py[j], span_patterns[span]);
        }
    }

    for (i = 0; i < POINTS; i++)
        draw_point(px[i], py[i]);

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
    statline(STATLINE_OFF);     /* a mode switch can re-enable the status line */
    if (gfx_mode(MODE_HERCULES) < 0) {
        statline(STATLINE_ON);    
        fprintf(stderr, "Hercules graphics unsupported on this system.\n");
        return 1;
    }

    rose_demo();

    gfx_mode(MODE_TEXT);
    clrscr();
    statline(STATLINE_ON);   
}
