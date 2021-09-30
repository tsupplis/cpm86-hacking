#ifndef _CONIO_H_
#define _CONIO_H_


/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#define STATLINE_OFF 0
#define STATLINE_ON  1

#define CURSOR_OFF     0
#define CURSOR_ON      1
#define CURSOR_SAVE    2
#define CURSOR_RESTORE 3

#ifdef __LEGACY__
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
gotoxy();
color();
int cputs();
int cputc();
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
void gotoxy(int x, int y);
void textcolor(int fg);
int cputs(char *);
int cputc(char);
#endif

#endif
