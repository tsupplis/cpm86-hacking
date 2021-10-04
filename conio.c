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

#ifndef __STDC__
int cputc(c)
    char c;
#else
int cputc(char c)
#endif
{
	return bdos(2,c);
}

#ifndef __STDC__
int cputs(str)
    char *str;
#else
int cputs(char *str)
#endif
{
	while (*str) {
		bdos(2,*str++);
    }
	return 0;
}

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
        if(1) { 
            getch_buffer[(o+s)%GETCH_BUFLEN]=d;
            s++;
        }
    }
    return c;
}

#ifndef __STDC__
crtreset()
#else
void crtreset()
#endif
{
    if(osver()>0x22) {
        cputs("\x1bm\x1bq\x1bt\x1bu");
        return;
    }
    cputs("\x1bm\x1b0\x1b1\x1bq\x1bt\x1bu");
}

#ifndef __STDC__
clrscr()
#else
void clrscr()
#endif
{
    cputs("\x1bq\x1bE");
}

#ifndef __STDC__
clreos()
#else
void clreos()
#endif
{
    cputs("\x1bJ");
}

#ifndef __STDC__
clreol()
#else
void clreol()
#endif
{
        cputs("\x1bK");
}

#ifndef __STDC__
delchar()
#else
void delchar()
#endif
{
        cputs("\x1bN");
}

#ifndef __STDC__
insline()
#else
void insline()
#endif
{
        cputs("\x1bL");
}

#ifndef __STDC__
delline()
#else
void delline()
#endif
{
        cputs("\x1bM");
}

#ifndef __STDC__
gotoxy(x, y)
    int x;
    int y;
#else
void gotoxy(int x, int y)
#endif
{
    char msg[5];
    msg[0]=27;
    msg[1]='Y';
    msg[2]=x+32;
    msg[3]=y+32;
    msg[4]=0;
    cputs(msg);
}

#ifndef __STDC__
cursor(cmd)
    int cmd;
#else
void cursor(int cmd)
#endif
{
    if(osver()>0x22) {
        return;
    }
    switch(cmd) {
    case CURSOR_ON:
        cputs("\x1bm");
        break;
    case CURSOR_OFF:
        cputs("\x1bn");
        break;
    case CURSOR_SAVE:
        cputs("\x1bj");
        break;
    case CURSOR_RESTORE:
        cputs("\x1bk");
        break;
    }
}

#ifndef __STDC__
wrapline(on)
    int on;
#else
void wrapline(int on)
#endif
{
    if(on) {
        cputs("\x1bv");
    } else {
        cputs("\x1bw");
    }
}

#ifndef __STDC__
statline(on)
    int on;
#else
void statline(int on)
#endif
{
    if(osver()>0x22) {
        return;
    }
    if(on) {
        cputs("\x1b1");
    } else {
        cputs("\x1b0");
    }
}

#ifndef __STDC__
textcolor(fg)
    int fg;
#else
void textcolor(int fg)
#endif
{
    unsigned char msg[8];
    if(fg<1) {
        return;
    }
    msg[0]=27;
    msg[1]='j';
    msg[2]=27;
    msg[3]='b';
    msg[4]=((char)fg);
    msg[5]=27;
    msg[6]='k';
    msg[7]=0;
    cputs(msg);
}
