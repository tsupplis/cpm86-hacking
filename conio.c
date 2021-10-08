/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#include <ctype.h>
#include <conio.h>

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
        cputs("\x1be\x1bz");
    } else {
        cputs("\x1bm\x1b0\x1b1\x1bq\x1bt\x1bu");
    }
}

#ifndef __STDC__
clrscr()
#else
void clrscr()
#endif
{
    cputs("\x1bE");
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
    switch(cmd) {
    case CURSOR_ON:
        if(osver()>0x22)
            cputs("\x1be");
        else
            cputs("\x1bm");
        break;
    case CURSOR_OFF:
        if(osver()>0x22)
            cputs("\x1bf");
        else
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
scrmode(m)
    int m;
#else
void scrmode(int m)
#endif
{
    if(osver()>0x22) {
        return;
    }
    switch(m) {
    case SCRMODE_DEFAULT:
        crtreset();
        clrscr();
        break;
    case SCRMODE_COL80:
#asm
        push ax
        push bx
        push cx
        push dx
        mov al, 3
        mov ah, 0
        int 10h
        pop dx
        pop cx
        pop bx
        pop ax
#endasm
        statline(STATLINE_OFF);
        statline(STATLINE_ON);
        clrscr();
        break;
    case SCRMODE_COL40:
#asm
        push ax
        push bx
        push cx
        push dx
        mov ax, 0
        int 10h
        pop dx
        pop cx
        pop bx
        pop ax
#endasm
        statline(STATLINE_OFF);
        clrscr();
        break;
    case SCRMODE_MONO:
        cputs("\x1by");
        break;
    case SCRMODE_COLOR:
        cputs("\x1bx");
        break;
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
    msg[4]=((unsigned char)fg);
    msg[5]=27;
    msg[6]='k';
    msg[7]=0;
    cputs((char*)msg);
}


#ifndef __STDC__
setstatus(arg)
    char* arg;
#else
void setstatus(char* arg)
#endif
{
    int i=0;
    int es;
    int bx;
    int ax;
    int segs[4];
    int escape=0;
    int upper=0;

    if(osver()>0x22) {
        return;
    }
    segread(segs);
    ax=bdosx(49,0,&es,&bx);
    while(i<14) {
        if(*arg=='\\') {
            escape=1;
            arg++;
            continue;
        }
        if(*arg) {
            char c;
            c=*arg;
            if(escape) {
                escape=0;
                switch(c) {
                    case 'u':
                    case 'U':
                        upper=1;
                        c=0;
                        break;
                    case 'l':
                    case 'L':
                        upper=0;
                        c=0;
                        break;
                    case 's':
                    case 'S':
                        c=' ';
                        break;
                    case '\\':
                        c=' ';
                        break;
                    default:
                        c=0;
                        break;
                }
            } 
            if(!c) {
                arg++;
                continue;
            }
            if(c<' ') {
                c=' ';
            }
            if(isalpha(c) && !upper) {
                pokeb(bx+0x32+i,es,tolower(c));
            } else {
                pokeb(bx+0x32+i,es,c);
            }
            i++;
            arg++;
        } else {
            pokeb(bx+0x32+i,es,' ');
            i++;
        }
    }
}
