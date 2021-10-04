/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <os.h>


#define COL40 "\x1b0\x1b\xE" 
#define COL80 "\x1b0\x1b1\x1bE" 
#define SCRDEF "\x1bm\x1b0\x1b1\x1bq\x1bt\x1bu\x1bE" 
#define MONO "\x1by"
#define COLOR "\x1bx"

typedef struct _cmd_t {
    char * name;
    char * vt_cmd;
    char * desc;
    int (*fn)();
    int prefix;
} _cmd_t;

#ifndef __STDC__
int status(arg)
    char* arg;
#else
int status(char* arg)
#endif
{
    int i=0;
    int es;
    int bx;
    int ax;
    int segs[4];
    int escape=0;
    int upper=0;

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
    return 0;
}

#ifndef __STDC__
int col80(arg)
    char* arg;
#else
int col80(char* arg)
#endif
{
    fprintf(stdout,COLOR);
#asm
    push ax
    push bx
    push cx
    push dx
    mov al, 3
    mov ah, 0
    int 16
    pop dx
    pop cx
    pop bx
    pop ax
#endasm
    return 0;
}

#ifndef __STDC__
int cls(arg)
    char* arg;
#else
int cls(char* arg)
#endif
{
    clrscr();
    return 0;
}

#ifndef __STDC__
int cursor_on(arg)
    char* arg;
#else
int cursor_on(char* arg)
#endif
{
    cursor(CURSOR_ON);
    return 0;
}

#ifndef __STDC__
int cursor_off(arg)
    char* arg;
#else
int cursor_off(char* arg)
#endif
{
    cursor(CURSOR_OFF);
    return 0;
}

#ifndef __STDC__
int statline_on(arg)
    char* arg;
#else
int statline_on(char* arg)
#endif
{
    statline(STATLINE_ON);
    return 0;
}

#ifndef __STDC__
int statline_off(arg)
    char* arg;
#else
int statline_off(char* arg)
#endif
{
    statline(STATLINE_OFF);
    return 0;
}

#ifndef __STDC__
int fg(arg)
    char* arg;
#else
int fg(char* arg)
#endif
{
    char fg=255;
    if(strlen(arg)>1) {
        return -1;
    }
    if(arg[0]>='1' && arg[0]<='9') {
        fg=arg[0]-'0';       
    } else if(arg[0]>='a' && arg[0]<='f') {
        fg=arg[0]-'a'+10;       
    } else if(arg[0]>='A' && arg[0]<='F') {
        fg=arg[0]-'A'+10;       
    } else {
        return -1;
    }
    fprintf(stdout,"\x1bb%c",fg);
    return 0;
}

#ifndef __STDC__
int bg(arg)
    char* arg;
#else
int bg(char* arg)
#endif
{
    char bg=255;
    if(strlen(arg)>1) {
        return -1;
    }
    if(arg[0]>='0' && arg[0]<='9') {
        bg=arg[0]-'0';       
    } else if(arg[0]>='a' && arg[0]<='f') {
        bg=arg[0]-'a'+10;       
    } else if(arg[0]>='A' && arg[0]<='F') {
        bg=arg[0]='A'+10;
    } else {
        return -1;
    }
    fprintf(stdout,"\x1bc%c",bg);
    return 0;
}

#ifndef __STDC__
int col40(arg)
    char* arg;
#else
int col40(char* arg)
#endif
{
    fprintf(stdout,COLOR);
#asm
    push ax
    push bx
    push cx
    push dx
    mov ax, 0
    int 16
    pop dx
    pop cx
    pop bx
    pop ax
#endasm
    return 0;
}

_cmd_t cmds[]= {
    {"cls", 0, "Clear screen",cls, 0 },
    {"cursor=on", 0, "Show cursor", cursor_on, 0 },
    {"cursor=off", 0, "Hide cursor", cursor_off, 0 },
    {"line=on", 0, "Show status line", statline_on, 0 },
    {"line=off", 0, "Hide status line", statline_off, 0 },
    {"status=", 0, "Set status line message \n          (\\s: space, \\\\: \\, \\u:upper, \\l:lower)", status, 1 },
    {"col=40", COL40, "Switch to 40 columns", col40, 0 },
    {"col=80", COL80, "Switch to 80 columns", col80, 0 },
    {"mono", MONO, "Switch to Mono", 0, 0 },
    {"color", COLOR, "Switch to Color", 0, 0 },
    {"fg=", 0, "Set foreground color (1-F)", fg, 1 },
    {"bg=", 0, "Set background color (1-F)", bg, 1 },
    {"scrdef", SCRDEF, "Default screen settings", 0, 0 },
    {0,0,0,0}
};

#ifndef __STDC__
int strlowercmp (p1, p2, p)
    char *p1;
    char *p2;
    int p;
#else
int strlowercmp (char *p1, char *p2,int p)
#endif
{
  unsigned char *s1 = (unsigned char *) p1;
  unsigned char *s2 = (unsigned char *) p2;
  unsigned char c1, c2;

  do
    {
      c1 = (unsigned char) tolower(*s1++);
      c2 = (unsigned char) tolower(*s2++);
      if (c1 == '\0')
        if(p) {
            return 0;
        } else {
            return c1 - c2;
        }
    }
  while (c1 == c2);

  return c1 - c2;
}

#ifndef __STDC__
int main(argc, argv) 
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
    int index=0;
    int done=0;
    int arg=1;

    if(osver()>0x22) {
        fprintf(stderr, "ERR: Requires CP/M-86 1.1\n");
        exit(-1);
    }
    if(argc<2) {
        exit(0);
    }
    if(!strcmp(argv[1],"-h")||!strcmp(argv[1],"-H")) {
        fprintf(stderr, "INF: Usage: mode -h | option option ...\n");
        fprintf(stderr, "INF:     -h for help\n");
        fprintf(stderr, "INF: and options:\n");
        index=0;
        while(cmds[index].name) {
            fprintf(stderr, "INF:      %-10s %s\n",cmds[index].name,cmds[index].desc);
            index++;
        }
        exit(0);
    }
    freopen("con:", "w", stdout);
    while(arg<argc) {
        done=0;
        index=0;
        while(cmds[index].name) {
            if(!strlowercmp(cmds[index].name,argv[arg],cmds[index].prefix)) {
                done=1;
                if(cmds[index].fn) {
                    if(cmds[index].fn(argv[arg]+strlen(cmds[index].name))) {
                        fprintf(stderr, "ERR: Action(%s) failed\n",argv[arg]);
                        exit(-1);
                    }
                }
                if(cmds[index].vt_cmd) {
                    printf("%s",cmds[index].vt_cmd);
                } 
                break;
            }
            index++;
        }
        if(!done) {
            fprintf(stderr, "ERR: Action(%s) unknown\n",argv[arg]);
            exit(-1);
        }
        arg++;
    }
    exit(0);
}

