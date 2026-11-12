#!/usr/bin/env python3
"""Check ripple visibility and capture the actual C renderer on the host."""
from pathlib import Path
import os
import subprocess
import tempfile
import sys

ROOT = Path(__file__).resolve().parents[1]
HARNESS = r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define main ripple_main
#include "ripple.c"
#undef main
static unsigned char pixels[480][640];
static int plotted;
void gfx_pixel(int x, int y, int color) {
    assert(x>=0 && x<640 && y>=0 && y<480);
    assert(color>0 && color<16);
    pixels[y][x]=(unsigned char)color; plotted++;
}
int gfx_mode(int mode) { return 0; }
void gfx_text(int col, int row, char *text, int color) {}
int kbhit(void) { return 1; }
int getch(void) { return 0; }
void clrscr(void) {}
int main(int argc, char **argv) {
    int x, y;
    FILE *f;
    static unsigned char palette[16][3] = {
        {0,0,0},{0,0,170},{0,170,0},{0,170,170},
        {170,0,0},{170,0,170},{170,85,0},{170,170,170},
        {85,85,85},{85,85,255},{85,255,85},{85,255,255},
        {255,85,85},{255,85,255},{255,255,85},{255,255,255}
    };
    for(x=0;x<640;x++) horizon[x]=next_horizon[x]=200;
    meshline(10,210,30,210,15);
    assert(plotted==0); /* A rear line below the foreground is invisible. */
    meshline(10,190,30,190,15);
    assert(plotted==21 && next_horizon[20]==190 && horizon[20]==200);
    meshline(-20,-20,-1,-1,15);
    assert(plotted==21);
    memset(pixels,0,sizeof pixels); plotted=0;
    prepare();
    assert(heightmap[0][0]==119);
    assert(radial[0]>radial[98] && radial[98]>radial[195]);
    ripple_draw();
    assert(plotted>10000);
    if(argc>1) {
        f=fopen(argv[1],"wb"); assert(f);
        fprintf(f,"P6\n640 480\n255\n");
        for(y=0;y<480;y++) for(x=0;x<640;x++)
            fwrite(palette[pixels[y][x]],1,3,f);
        assert(fclose(f)==0);
    }
    puts("PASS: ripple bounds, hidden lines, slice isolation and outward decay");
    return 0;
}
'''
with tempfile.TemporaryDirectory(prefix='ripple-check-') as folder:
    work = Path(folder)
    (work / 'conio.h').write_text('int kbhit(void); int getch(void); void clrscr(void);\n')
    (work / 'check.c').write_text(HARNESS)
    subprocess.run([os.environ.get('CC_HOST', 'cc'), '-std=c89',
                    '-I'+str(work), '-I'+str(ROOT), str(work/'check.c'),
                    '-lm', '-o', str(work/'check')], check=True)
    subprocess.run([str(work/'check'), *sys.argv[1:]], check=True)
