/*
     The contents of this file are subject to the Mozilla Public License
     Version 1.0 (the "License"); you may not use this file except in
     compliance with the License. You may obtain a copy of the License at
     http://www.mozilla.org/MPL/

     Software distributed under the License is distributed on an "AS IS"
     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
     License for the specific language governing rights and limitations
     under the License.
*/

#include <stdio.h>
#include <ctype.h>


#define CLS "\x1b\x45" 
#define CURSOROFF "\x1b\x6E" 
#define CURSORON "\x1b\x6D" 
#define LINEOFF "\x1b\x30" 
#define LINEON "\x1b\x31" 
#define COL40 "\x1b\x30\x1b\x45" 
#define COL80 "\x1b\x30\x1b\x31\x1b\x45" 

typedef struct _cmd_t {
    char * name;
    char * vt_cmd;
    char * desc;
    int (*fn)();
} _cmd_t;

#ifdef __LEGACY__
int col80(arg)
    char* arg;
#else
int col80(char* arg)
#endif
{
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

#ifdef __LEGACY__
int col40(arg)
    char* arg;
#else
int col40(char* arg)
#endif
{
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
    {"cls", CLS, "Clear screen",0 },
    {"cursor=on", CURSORON, "Show cursor", 0 },
    {"cursor=off", CURSOROFF, "Hide cursor", 0 },
    {"line=on", LINEON, "Show status line", 0 },
    {"line=off", LINEOFF, "Hide status line", 0 },
    {"col=40", COL40, "Switch to 40 columns", col40 },
    {"col=80", COL80, "Switch to 80 columns", col80 },
    {0,0,0,0}
};

#ifdef __LEGACY__
int strlowercmp (p1, p2)
    char *p1;
    char *p2;
#else
int strlowercmp (char *p1, char *p2)
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
        return c1 - c2;
    }
  while (c1 == c2);

  return c1 - c2;
}

#ifdef __LEGACY__
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
    while(arg<argc) {
        done=0;
        index=0;
        while(cmds[index].name) {
            if(!strlowercmp(cmds[index].name,argv[arg])) {
                done=1;
                if(cmds[index].fn) {
                    if(cmds[index].fn(argv[arg])) {
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

