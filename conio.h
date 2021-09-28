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
clreol();
clreos();
delline();
insline();
delchar();
cursor();
statline();
gotoxy();
color();
#else
int kbhit();
int getch();
void clrscr();
void clreol();
void clreos();
void delline();
void insline();
void delchar();
void cursor(int on);
void statline(int on);
int gotoxy(int x, int y);
void color(int fg, int bg);
#endif

#endif
