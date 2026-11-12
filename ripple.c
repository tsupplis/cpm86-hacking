/* Licensed under the MIT license. See LICENSE in the project root. */
#include <stdio.h>
#include <math.h>
#include <conio.h>
#include "gfx.h"

#define WIDTH 640
#define HEIGHT 480
#define GRID 32
#define CENTER_Y 260

/* One quadrant suffices: the ripple is symmetric about both axes. */
static int heightmap[GRID+1][GRID+1];
static int radial[364];
static int horizon[WIDTH];
static int next_horizon[WIDTH];

static int shade(int height)
{
    if (height > 65) return 15;
    if (height > 35) return 14;
    if (height > 15) return 10;
    if (height > 0) return 11;
    if (height > -15) return 3;
    if (height > -35) return 9;
    return 1;
}

/* Draw against the completed foreground, accumulating this slice separately.
 * A single upper horizon hides the underside as well as the rear mesh.
 * No frame buffer or graphics readback is required. */
static void meshline(int x0, int y0, int x1, int y1, int color)
{
    int dx, dy, sx, sy, err, twice;
    dx = x1-x0; dy = y1-y0;
    sx = dx < 0 ? -1 : 1; sy = dy < 0 ? -1 : 1;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    err = dx-dy;
    for (;;) {
        if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0 < HEIGHT) {
            if (y0 < horizon[x0]) gfx_pixel(x0, y0, color);
            if (y0 < next_horizon[x0]) next_horizon[x0] = y0;
        }
        if (x0 == x1 && y0 == y1) break;
        twice = err*2;
        if (twice > -dy) { err -= dy; x0 += sx; }
        if (twice < dx) { err += dx; y0 += sy; }
    }
}

static void prepare(void)
{
    int i, j, radius;
    double r;
    for (i = 0; i < 364; i++) {
        r = (double)i/51.2;
        radial[i] = (int)(119.0*cos(3.3*r)*exp(-0.55*r));
    }
    for (j = 0; j <= GRID; j++)
        for (i = 0; i <= GRID; i++) {
            radius = (int)(8.0*sqrt((double)(i*i+j*j))+0.5);
            heightmap[j][i] = radial[radius];
        }
}

static void ripple_draw(void)
{
    int i, j, ax, ay, x, y, last_y, last_h, h;
    for (x = 0; x < WIDTH; x++) horizon[x] = HEIGHT;
    /* Front to back: nearer folds establish the visibility boundary. */
    for (j = GRID; j >= -GRID; j--) {
        ay = j < 0 ? -j : j;
        for (x = 0; x < WIDTH; x++) next_horizon[x] = horizon[x];
        last_y = last_h = 0;
        for (i = -GRID; i <= GRID; i++) {
            ax = i < 0 ? -i : i;
            h = heightmap[ay][ax];
            x = WIDTH/2+i*8;
            y = CENTER_Y+j*3-h;
            if (i != -GRID)
                meshline(x-8, last_y, x, y, shade((h+last_h)/2));
            last_y = y; last_h = h;
        }
        for (x = 0; x < WIDTH; x++) horizon[x] = next_horizon[x];
    }
}

int main(void)
{
    freopen("con:", "r", stdin);
    freopen("con:", "w", stdout);
    puts("Calculating ripple...");
    fflush(stdout);
    prepare();
    if (gfx_mode(MODE_VGA_640X480) < 0) {
        fprintf(stderr, "Graphics mode unsupported on this system.\n");
        return 1;
    }
    gfx_text(23, 0, "Ripple: a drop on a quiet surface", 15);
    ripple_draw();
    gfx_text(29, 28, "Press any key to exit", 7);
    while (!kbhit()) ;
    getch();
    gfx_mode(MODE_TEXT);
    clrscr();
    return 0;
}
