/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <util.h>

/* Internal assembly helper: use the installed DOS Plus INT 10h hook. */
int dosplus_crtreset();

#ifdef __STDC__
#include <stdlib.h>
#else
char * malloc();
#endif

typedef struct terminal_key {
    unsigned char *bytes;
    unsigned int length;
    int key;
} terminal_key_t;

typedef struct terminal_output {
    char *reset[2];
    char *clear_screen;
    char *clear_eos;
    char *clear_eol;
    char *insert_line;
    char *delete_line;
    char *delete_char;
    char *cursor_on[2];
    char *cursor_off[2];
    char *cursor_save;
    char *cursor_restore;
    char *wrap_on;
    char *wrap_off;
    char *status_on;
    char *status_off;
    char *mono;
    char *color;
    char *position_prefix;
    unsigned char position_bias;
    char *foreground_prefix;
    char *foreground_suffix;
} terminal_output_t;

typedef struct terminal_input {
    terminal_key_t *keys;
    unsigned int key_count;
} terminal_input_t;

typedef struct terminal_cfg {
    terminal_output_t output;
    terminal_input_t input;
} terminal_cfg_t;

/* VT52 console data; paired entries select early/later OS variants.
 * Raw byte input has no key decoder or verified terminal key map yet. */
static terminal_cfg_t terminal = {
    {
        {"\033m\0330\0331\033q\033t\033u", "\033e\033z"},
        "\x1bE", "\x1bJ", "\x1bK",
        "\x1bL", "\x1bM", "\x1bN",
        {"\x1bm", "\x1be"}, {"\x1bn", "\x1bf"},
        "\x1bj", "\x1bk",
        "\x1bv", "\x1bw",
        "\x1b1", "\x1b0",
        "\x1by", "\x1bx",
        "\033Y", 32,
        "\033j\033b", "\033k"
    },
    {0, 0}
};


int kbhit() {   
    return bdos(6,254);
}

#define GETCH_BUFLEN (64)

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
        c=(unsigned char)getch_buffer[o];s--;o++;
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
    int version = osver();

    if(version == 0x41 && !(ostype() & OSTYPE_MULTIUSER))
        dosplus_crtreset();
    cputs(terminal.output.reset[version>0x22]);
}

#ifndef __STDC__
clrscr()
#else
void clrscr()
#endif
{
    cputs(terminal.output.clear_screen);
}

#ifndef __STDC__
clreos()
#else
void clreos()
#endif
{
    cputs(terminal.output.clear_eos);
}

#ifndef __STDC__
clreol()
#else
void clreol()
#endif
{
    cputs(terminal.output.clear_eol);
}

#ifndef __STDC__
delchar()
#else
void delchar()
#endif
{
    cputs(terminal.output.delete_char);
}

#ifndef __STDC__
insline()
#else
void insline()
#endif
{
    cputs(terminal.output.insert_line);
}

#ifndef __STDC__
delline()
#else
void delline()
#endif
{
    cputs(terminal.output.delete_line);
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
    int n=strlen(terminal.output.position_prefix);
    strcpy(msg,terminal.output.position_prefix);
    msg[n++]=x+terminal.output.position_bias;
    msg[n++]=y+terminal.output.position_bias;
    msg[n]=0;
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
        cputs(terminal.output.cursor_on[osver()>0x22]);
        break;
    case CURSOR_OFF:
        cputs(terminal.output.cursor_off[osver()>0x22]);
        break;
    case CURSOR_SAVE:
        cputs(terminal.output.cursor_save);
        break;
    case CURSOR_RESTORE:
        cputs(terminal.output.cursor_restore);
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
    cputs(on?terminal.output.wrap_on:terminal.output.wrap_off);
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
        ;
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
        ;
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
        cputs(terminal.output.mono);
        break;
    case SCRMODE_COLOR:
        cputs(terminal.output.color);
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
    int os=osver();
    if(os>0x31) {
        xstatline(on?2:1);
        return;
    }
    if((ostype() & OSTYPE_MULTIUSER)) {
        cputs(on?"\0333":"\0330");
        return;
    }
    if(os==0x22) {
        cputs(on?terminal.output.status_on:terminal.output.status_off);
        return;
    }
}

#ifndef __STDC__
textcolor(fg)
    int fg;
#else
void textcolor(int fg)
#endif
{
    char msg[8];
    int n;
    if(fg<1) {
        return;
    }
    n=strlen(terminal.output.foreground_prefix);
    strcpy(msg,terminal.output.foreground_prefix);
    msg[n++]=(unsigned char)fg;
    strcpy(msg+n,terminal.output.foreground_suffix);
    cputs(msg);
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
