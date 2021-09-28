/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#include "conio.h"

int kbhit() {   
    return bdos(6,254);
}

#define GETCH_BUFLEN (12)

static char getch_buffer[GETCH_BUFLEN];

int getch()
{
    int i,c,d;
    static int s=0;
    static int o=0;

    if(s>0) {
        c=getch_buffer[o];s--;o++;
        o=o%GETCH_BUFLEN;
        return c;
    }
    while(!(c=bdos(6,255))) 
        continue;
    while(s<GETCH_BUFLEN && (d=bdos(6,255))) {
        if(c!=27) { 
            getch_buffer[(o+s)%GETCH_BUFLEN]=d;
            s++;
        }
    }
    return c;
}

#ifdef __LEGACY__
clrscr()
#else
void clrscr()
#endif
{
    bdos(9,"\x1bE$");
}

#ifdef __LEGACY__
clreos()
#else
void clreos()
#endif
{
    bdos(9,"\x1bJ$");
}

#ifdef __LEGACY__
clreol()
#else
void clreol()
#endif
{
        bdos(9,"\x1bK$");
}

#ifdef __LEGACY__
delchar()
#else
void delchar()
#endif
{
        bdos(9,"\x1bN$");
}

#ifdef __LEGACY__
insline()
#else
void insline()
#endif
{
        bdos(9,"\x1bL$");
}

#ifdef __LEGACY__
delline()
#else
void delline()
#endif
{
        bdos(9,"\x1bM$");
}

#ifdef __LEGACY__
gotoxy(x, y)
    int x;
    int y;
#else
void gotoxy(int x, int y)
#endif
{
    char msg[6];
    strcpy(msg,"\x1BY..$");
    msg[2]=x+32;
    msg[3]=y+32;
    bdos(9,msg);
}

#ifdef __LEGACY__
cursor(cmd)
    int cmd;
#else
void cursor(int cmd)
#endif
{
    switch(cmd) {
    case CURSOR_ON:
        bdos(9,"\x1bm$");
        break;
    case CURSOR_OFF:
        bdos(9,"\x1bn$");
        break;
    case CURSOR_SAVE:
        bdos(9,"\x1bj$");
        break;
    case CURSOR_RESTORE:
        bdos(9,"\x1bk$");
        break;
    }
}

#ifdef __LEGACY__
statline(on)
    int on;
#else
void statline(int on)
#endif
{
    if(on) {
        bdos(9,"\x1b1$");
    } else {
        bdos(9,"\x1b0$");
    }
}

#ifdef __LEGACY__
color(fg,bg)
    int fg;
    int bg;
#else
void color(int fg, int bg);
#endif
{
    char msg[8];
    strcpy(msg,"\x1Bb.\x1Bc.$");
    msg[2]=fg;
    msg[4]=bg;
    bdos(9,msg);
}
