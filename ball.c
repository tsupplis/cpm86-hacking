/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/
#include <stdio.h>
#include <conio.h>
#include <os.h>
#include <gfx.h>

#define BALL_WIDTH 8
#define BALL_HEIGHT 8

int ball[BALL_WIDTH*BALL_HEIGHT]= {
    0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 1, 1, 1, 1, 0, 0, 
    0, 1, 2, 2, 3, 2, 1, 0, 
    0, 1, 2, 2, 3, 3, 1, 0, 
    0, 1, 2, 2, 2, 2, 1, 0, 
    0, 1, 2, 2, 2, 2, 1, 0, 
    0, 0, 1, 1, 1, 1, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0
};

#ifndef __STDC__
draw_map(x, y, map, width, height)
    int x;
    int y;
    int * map;
    int width;
    int height;
#else
void draw_map(int x, int y, int *map, int width, int height)
#endif
{
    int offset=0;
    int x_offset=0;
    int y_offset=0;
    int length=width*height;
    do {
        gfx_pixel(x + x_offset, y + y_offset, ball[offset]);
        offset++;
        x_offset++;
        x_offset=x_offset%width;
        if (!x_offset) {
            y_offset++;
        }
    } while (offset < length);
}

#ifndef __STDC__
box(x, y, width, height)
    int x;
    int y;
    int width;
    int height;
#else
void box(int x, int y, int width, int height)
#endif
{
    int curx = x;
    int cury = y;
    width += x;
    height += y;
    do {
        gfx_pixel(curx++, cury, 1);
    } while (curx != width);
    do {
        gfx_pixel(curx, cury++, 1);
    } while (cury != height);
    do {
        gfx_pixel(curx--, cury, 1);
    } while (curx != x);
    do {
        gfx_pixel(curx, cury--, 1);
    } while (cury != y);
}

#define BOX_X 89
#define BOX_Y 59
#define BOX_WIDTH 139
#define BOX_HEIGHT 78
#define BOX_THICKNESS 1


#ifndef __STDC__
ball_demo()
#else
void ball_demo
#endif
{
    int x = BOX_X+1;
    int y = BOX_Y+1;
    int xinc = 1;
    int yinc = 1;

    box(BOX_X, BOX_Y, BOX_WIDTH, BOX_HEIGHT);
    do {
        x += xinc;
        y += yinc;
        if (x > BOX_X+BOX_WIDTH-BALL_WIDTH-BOX_THICKNESS)
            xinc = -1;
        else if (x <= BOX_X+BOX_THICKNESS)
            xinc = 1;
        if (y > BOX_Y+BOX_HEIGHT-BALL_WIDTH-BOX_THICKNESS)
            yinc = -1;
        else if (y <= BOX_Y+BOX_THICKNESS)
            yinc = 1;
        delay(1);
        draw_map(x, y, ball, BALL_WIDTH, BALL_HEIGHT);
    } while (!kbhit());
    clrscr();
}

#ifndef __STDC__
int main(argc, argv) 
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
    int x = BOX_X+1;
    int y = BOX_Y+1;
    int xinc = 1;
    int yinc = 1;

    freopen("con:", "r", stdin);
    freopen("con:", "w", stdout);
    statline(STATLINE_OFF);
    gfx_mode(MODE_CGA_320X200);
    gfx_palette(CGA_PALETTE_GRY);
    ball_demo();
    gfx_mode(MODE_TEXT);
    return 0;
}
