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
int puts(str)
    register char *str;
#else
int puts(register char *str)
#endif
{
	while (*str)
		if (putchar(*str++) == -1)
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
    puts("\x1bm\x1b0\x1b1\x1bq\x1bt\x1bu");
}

#ifdef __LEGACY__
clrscr()
#else
void clrscr()
#endif
{
    puts("\x1bE");
}

#ifdef __LEGACY__
clreos()
#else
void clreos()
#endif
{
    puts("\x1bJ");
}

#ifdef __LEGACY__
clreol()
#else
void clreol()
#endif
{
        puts("\x1bK");
}

#ifdef __LEGACY__
delchar()
#else
void delchar()
#endif
{
        puts("\x1bN");
}

#ifdef __LEGACY__
insline()
#else
void insline()
#endif
{
        puts("\x1bL");
}

#ifdef __LEGACY__
delline()
#else
void delline()
#endif
{
        puts("\x1bM");
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
    strcpy(msg,"\x1BY..");
    msg[2]=x+32;
    msg[3]=y+32;
    puts(msg);
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
        puts("\x1bm");
        break;
    case CURSOR_OFF:
        puts("\x1bn");
        break;
    case CURSOR_SAVE:
        puts("\x1bj");
        break;
    case CURSOR_RESTORE:
        puts("\x1bk");
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
        puts("\x1b1");
    } else {
        puts("\x1b0");
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
    strcpy(msg,"\x1Bb.\x1Bc.");
    msg[2]=fg;
    msg[5]=bg;
    puts(msg);
}
