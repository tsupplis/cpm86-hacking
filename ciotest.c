#include "conio.h"
#include <stdio.h>
#include <ctype.h>

#ifdef __LEGACY__
int main(argc,argv)
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
	int i;
	int c;

	clrscr();
	statline(STATLINE_OFF);
    gotoxy(0,0);printf("(0,0)");
    gotoxy(0,74);printf("(0,79)");
    gotoxy(23,73);printf("(23,79");
    gotoxy(23,0);printf("(23,0)");
        getch();
	for(i=1;i<14;i++) {
		gotoxy(3+i,1);
        getch();
		color(1+i%6+1,9);
		printf("[%02d] Line Added ------------------------- ",i);
        getch();
	}
    gotoxy(23,28);printf("[press a key to continue]");
	c=getch();
    delline();
    crtreset();
	return 0;
}
