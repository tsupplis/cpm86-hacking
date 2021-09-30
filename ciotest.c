/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#include <ctype.h>
#include <conio.h>

#ifndef __STDC__
int main(argc,argv)
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
	int i;
	int c;
    int j;

	clrscr();
    cursor(CURSOR_OFF);
	statline(STATLINE_OFF);
    gotoxy(0,0);cputs("(0,0)");
    gotoxy(0,74);cputs("(0,79)");
    gotoxy(23,73);wrapline(WRAPLINE_OFF);cputs("(23,79)");wrapline(WRAPLINE_ON);
    gotoxy(23,0);cputs("(23,0)");
	for(i=0;i<10;i++) {
		gotoxy(3+i,1);
	    textcolor(2);
		printf("[%02d] Line Added ----------",i+1);
	    textcolor(4);
		printf("oooooooooooo");
	}
	c=getch();
    for(j=0;j<40;j++){
        for(i=0;i<10;i++) {
            gotoxy(3+i,1);
            textcolor(j%16);
		    printf("[%02d] Line Added ----------",i+1);
        }
        c=getch();
	}
	c=getch();
	textcolor(4);
    gotoxy(23,26);
    printf("[press a key to continue]");
	c=getch();
    delline();
    crtreset();
	return 0;
}
