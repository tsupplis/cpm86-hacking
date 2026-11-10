/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/
#include <stdio.h>
#include <math.h>
#include <conio.h>
#include <util.h>
#include <gfx.h>

#define SCREEN_WIDTH   640
#define SCREEN_HEIGHT  480

#define ORIGIN_X       320
#define ORIGIN_Y       248
#define RADIUS         208
#define POINTS         10
#define CIRCLE_STEPS   120

#define COLOR_CIRCLE   8   /* Dark Grey */
#define COLOR_POINT    15  /* Bright White */
#define COLOR_TEXT     15  /* Bright White */

#define PI             3.14159265358979323846

/* One colour per chord span: neighbours (1) out to diameters (POINTS / 2). */
static int span_colors[] = { 0, 9, 11, 10, 14, 12 };

static int px[POINTS];
static int py[POINTS];

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
        draw_line(prev_x, prev_y, x, y, COLOR_CIRCLE);
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
                gfx_pixel(x + dx, y + dy, COLOR_POINT);
}

#ifndef __STDC__
void rose_demo()
#else
void rose_demo(void)
#endif
{
    int i, j, span;
    double rad;

    gfx_text(31, 0, "Mystic rose: 10 points", COLOR_TEXT);
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
            draw_line(px[i], py[i], px[j], py[j], span_colors[span]);
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
    if (gfx_mode(MODE_VGA_640X480) < 0) {
        fprintf(stderr, "Graphics mode unsupported on this system.\n");
        return 1;
    }

    rose_demo();

    gfx_mode(MODE_TEXT);
    clrscr();
    return 0;
}
