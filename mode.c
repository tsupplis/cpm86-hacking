/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <util.h>


typedef struct _cmd_t {
    char * name;
    char * vt_cmd;
    char * desc;
#ifndef __STDC__
    int (*fn)();
#else
    int (*fn)(char *);
#endif
    int prefix;
    int cpm86;
} _cmd_t;

#ifndef __STDC__
int status(arg)
    char* arg;
#else
int status(char* arg)
#endif
{
    setstatus(arg);
}

#ifndef __STDC__
int set_mode(arg)
    char* arg;
#else
int set_mode(char* arg)
#endif
{
    if(!istrcmp("default",arg,0)) {
        scrmode(SCRMODE_DEFAULT);
    } else if(!istrcmp("col80",arg,0)) {
        scrmode(SCRMODE_COL80);
    } else if(!istrcmp("col40",arg,0)) {
        scrmode(SCRMODE_COL40);
    } else if(!istrcmp("color",arg,0)) {
        scrmode(SCRMODE_COLOR);
    } else if(!istrcmp("mono",arg,0)) {
        scrmode(SCRMODE_MONO);
    } else {
        return -1;
    }
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

_cmd_t cmds[]= {
    {"cls", 0, "Clear screen",cls, 0, 0x2000},
    {"cursor=on", 0, "Show cursor", cursor_on, 0 , 0x2000},
    {"cursor=off", 0, "Hide cursor", cursor_off, 0, 0x2000},
    {"statln=on", 0, "Show status line", statline_on, 0, 0x22},
    {"statln=off", 0, "Hide status line", statline_off, 0, 0x22},
    {"status=", 0, "Set status line message", status, 1, 0x22},
    {"scr=", 0, "Set screen mode (default,col80,col40,mono,color)", set_mode, 1, 0x22},
    {"fg=", 0, "Set foreground color (1-F)", fg, 1, 0x2000},
    {"bg=", 0, "Set background color (1-F)", bg, 1, 0x2000},
    {0,0,0,0}
};


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
    int os=osver();

    if(argc<2) {
        exit(0);
    }
    if(!strcmp(argv[1],"-h")||!strcmp(argv[1],"-H")) {
        fprintf(stderr, "INF: Usage: mode -h | option option ...\n");
        fprintf(stderr, "INF: Console configuration utility\n");
        fprintf(stderr, "INF:     -h for help\n");
        fprintf(stderr, "INF: and options:\n");
        index=0;
        while(cmds[index].name) {
            if(os>cmds[index].cpm86) {
                index++;
                continue;
            }
            fprintf(stderr, "INF:      %-10s   %s\n",cmds[index].name,cmds[index].desc);
            index++;
        }
        exit(0);
    }
    freopen("con:", "w", stdout);
    while(arg<argc) {
        done=0;
        index=0;
        while(cmds[index].name) {
            if(!istrcmp(cmds[index].name,argv[arg],cmds[index].prefix)) {
                if(os>cmds[index].cpm86) {
                    index++;
                    continue;
                }
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

