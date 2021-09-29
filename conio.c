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

#ifdef __LEGACY__
int cputs(str)
    register char *str;
#else
int cputs(register char *str)
#endif
{
	while (*str)
		bdos(2,*str++);
			return -1;
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
        if(c!=27) { 
            getch_buffer[(o+s)%GETCH_BUFLEN]=d;
            s++;
        }
    }
    return c;
}

#ifdef __LEGACY__
crtreset()
#else
void crtreset()
#endif
{
    cputs("\x1bm\x1b0\x1b1\x1bq\x1bt\x1bu");
}

#ifdef __LEGACY__
clrscr()
#else
void clrscr()
#endif
{
    cputs("\x1bq\x1bE");
}

#ifdef __LEGACY__
clreos()
#else
void clreos()
#endif
{
    cputs("\x1bJ");
}

#ifdef __LEGACY__
clreol()
#else
void clreol()
#endif
{
        cputs("\x1bK");
}

#ifdef __LEGACY__
delchar()
#else
void delchar()
#endif
{
        cputs("\x1bN");
}

#ifdef __LEGACY__
insline()
#else
void insline()
#endif
{
        cputs("\x1bL");
}

#ifdef __LEGACY__
delline()
#else
void delline()
#endif
{
        cputs("\x1bM");
}

#ifdef __LEGACY__
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

#ifdef __LEGACY__
cursor(cmd)
    int cmd;
#else
void cursor(int cmd)
#endif
{
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

#ifdef __LEGACY__
statline(on)
    int on;
#else
void statline(int on)
#endif
{
    if(on) {
        cputs("\x1b1");
    } else {
        cputs("\x1b0");
    }
}

#ifdef __LEGACY__
textcolor(fg)
    int fg;
#else
void textcolor(int fg)
#endif
{
    char msg[5];
    if(fg>8) fg=8;
    if(fg<1) fg=1;
    msg[0]=27;
    msg[1]='b';
    msg[2]=fg;
    msg[3]=0;
    cputs(msg);
}
