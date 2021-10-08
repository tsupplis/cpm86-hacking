#ifndef _CONIO_H_
#define _CONIO_H_


/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#define WRAPLINE_OFF 0
#define WRAPLINE_ON  1

#define STATLINE_OFF 0
#define STATLINE_ON  1

#define CURSOR_OFF     0
#define CURSOR_ON      1
#define CURSOR_SAVE    2
#define CURSOR_RESTORE 3

#define SCRMODE_DEFAULT 0 
#define SCRMODE_COL80   1
#define SCRMODE_COL40   2
#define SCRMODE_MONO    4
#define SCRMODE_COLOR   8

#ifndef __STDC__
int kbhit();
int getch();
clrscr();
crtreset();
clreol();
clreos();
delline();
insline();
delchar();
cursor();
statline();
wrapline();
gotoxy();
textcolor();
setstatus();
int cputs();
int cputc();
scrmode();
#else
int kbhit();
int getch();
void clrscr();
void crtreset();
void clreol();
void clreos();
void delline();
void insline();
void delchar();
void cursor(int on);
void statline(int on);
void wrapline(int on);
void gotoxy(int x, int y);
void textcolor(int fg);
void setstatus(char * s);
int cputs(char *);
int cputc(char);
void scrmode(int);
#endif

#endif
