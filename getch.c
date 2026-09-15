#include <stdio.h>
#include <ctype.h>
#include <conio.h>

#define GETCH_BUFLEN (64)

static char * getch_buffer=0;

#ifdef __DOS__
int dos_key() {
#asm
    push cx
    push bx
    push sp     ; Pushes the original SP value (before AX was pushed)
    push bp
    push si
    push di
    mov dl, 255
	mov ah, 06
	int 21h
    je nokey
    xor ah,ah
    jmp done
nokey:
    mov ax,0FFFFh
done:
	pop di
    pop si
    pop bp
    add sp, 2   ; Discards the saved SP value (replaces POP SP)
    pop bx
    pop cx
#endasm
}

int getch()
{
    int i,c,d;
    static int s=0;
    static int o=0;

    if(getch_buffer==0) {
        getch_buffer=(char*)malloc(GETCH_BUFLEN+1);
    }
    if(s>0) {
        printf(".");
        c=getch_buffer[o];s--;o++;
        o=o%GETCH_BUFLEN;
        return c;
    }
    while((c=dos_key())==0xFFFF) 
        continue;
    while(s<GETCH_BUFLEN && (d=dos_key())!=0xFFFF) {
        if(1) { 
            getch_buffer[(o+s)%GETCH_BUFLEN]=d;
            s++;
        }
    }
    return c;
}

#endif

#if defined(__CPM86__)

int getch()
{
    int i,c,d;
    static int s=0;
    static int o=0;

    if(getch_buffer==0) {
        getch_buffer=(char*)malloc(GETCH_BUFLEN+1);
    }
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
#endif

#ifndef __STDC__
int main(argc, argv) 
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
	int c;
    int fnkey=0;
    freopen("con:", "r", stdin);
    freopen("con:", "w", stdout);
    fprintf (stderr,"INF: Keyboard Scanner. type letter 'q' to leave ...\n");
	while(1) {
		c=getch();
        if(fnkey) {
            fprintf(stderr, "INF: 0x%02x -> fn%03d\n",c,c);
            fnkey=0;
        } else {
            if(c==32) {
                fprintf(stderr, "INF: 0x%02x [%-3d] -> 'space'\n",c,c);
#if defined(__DOS__)
            } else if(c==0x00 || c==0x0E) {
                fprintf(stderr, "INF: 0x%02x [%-3d] -> 'fnkeynext'\n",c,c);
                fnkey=1;
#endif
            } else if(isprint(c)) {
                fprintf(stderr, "INF: 0x%02x [%-3d] -> %c\n",c,c,c);
            } else if(c>=0 && c<' ') {
                fprintf(stderr, "INF: 0x%02x [%-3d] -> ^%c\n",c,c,c+'A'-1);
            } else {
                fprintf(stderr, "INF: 0x%02x [%-3d]\n",c,c);
            }
            if(c=='q' && !fnkey) {
                fprintf (stderr,"INF: Bye ...\n");
                break;
            }
        }
	}
	return 0;
}
