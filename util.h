#ifndef _UTIL_H_
#define _UTIL_H_

/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

typedef struct dpb_t {
    unsigned int spt;
    unsigned char bsh;
    unsigned char blm;
    unsigned char exm;
    unsigned int dsm;
    unsigned int drm;
    unsigned char al0;
    unsigned char al1;
    unsigned int cks;
    unsigned int off;
} dpb_t;

/*
 * time.c -- system clock, file timestamps, and Unix epoch conversion.
 *
 * CP/M-86 1.1: clock lives in sysdata area (BDOS 31h + 20h) as
 * "MM/DD/YY<sep>HH:MM:SS".  MP/M II and later: BDOS fn 105/104 (T_GET/T_SET).
 * File stamps (getfdatetime/setfdatetime) use BDOS fn 102/116.
 * YY 00-99 means 2000-2099.  Times are treated as UTC.
 */

struct sysdt {
    int year;   /* 2000..2099 */
    int mon;    /* 1..12      */
    int day;    /* 1..31      */
    int hour;   /* 0..23      */
    int min;    /* 0..59      */
    int sec;    /* 0..59      */
};

#define EPOCH_ERR  (-1L)    /* returned by now()/epoch() on error */


#define OSTYPE_MULTIUSER 0x04

#ifndef __STDC__
int keyb();
int bioskey();
int pckey();
int osver();
int ostype();
int getcurdrv();
int setcurdrv();
int getccpdrv();
int bdosx();
int getftime();
int setftime();
unsigned int getpcode();
unsigned int setpcode();
unsigned int getcmode();
unsigned int setcmode();
int gettime();
int settime();
int p_cli();
int ms_p_cli();
int p_priority();
int conattach();
int condetach();
unsigned int getpid();
unsigned int getppid();
int getpriority();
int getppriority();
xstatline();
int istrcmp();
char * getenv();
printenv();
delay();
dpb_load();
typedef unsigned int size_t;
#else
int keyb(void);
int bioskey(void);
int pckey(void);
int osver(void);
int ostype(void);
int getcurdrv(void);
int setcurdrv(int drv);
int getccpdrv(void);
int bdosx(int cx, int dx, int* es, int *bx);
int getftime(char *fcb);
int setftime(char *fcb, char *stamps);
unsigned int getpcode(void);
unsigned int setpcode(unsigned int code);
unsigned int getcmode(void);
unsigned int setcmode(unsigned int mode);

int gettime(char *stamp);
int settime(char *stamp);
int getdatetime(struct sysdt *dt);
int setdatetime(struct sysdt *dt);
int getfdatetime(char *fcb, struct sysdt *dt, int which);
int setfdatetime(char *fcb, struct sysdt *create_dt, struct sysdt *update_dt);
long epoch(struct sysdt *dt);
long epoch_now(void);

int p_cli(char *cmd);
/* CDOS 4.1 only: opened COM/EXE FCB, full command, synchronous. See API.md. */
int ms_p_cli(char *opened_fcb, char *cmd);
int p_priority(int priority);
int conattach(void);
int condetach(void);
unsigned int getpid(void);
unsigned int getppid(void);
int getpriority(void);
int getppriority(void);
void xstatline(int mode);
int istrcmp(char *p1, char *p2, int p);
char *getenv(const char *name);
void printenv(FILE * fp);
void delay(unsigned int delay);
void dpb_load(int drive, dpb_t*dpb);
#endif

#ifndef __STDC__
char *malloc();
#endif

/*
  Already provided by the Aztec C base library (not in util.lib):
    fcbinit(name,fcb)                -- "[user/][d:]name.ext" -> FCB; returns the user
                                        number, 255 = no prefix, -1 = bad drive
                                        (clears FCB bytes 0..15 only)
    getusr() / setusr(n) / rstusr()  -- BDOS fn 32 get/set user number; setusr(255)
                                        only saves, rstusr restores (one level)
    bdos(fn, dx)                     -- generic BDOS call, returns AX & 0xFF

  New in util.lib (os.asm), all version-checked via BDOS fn 12:
    getftime(fcb)         fn 102  version >= 30h; returns 0..3 ok, 0xFF fail/unsupported.
                                  Stamps land in the FCB: +18h create/access,
                                  +1Ch update, +0Ch password mode.
    setftime(fcb,stamps)  fn 116  CP/M-86 v4 only (version >= 41h); 0..3 ok, 0xFF fail.
                                  stamps = 8 bytes: create/access then update,
                                  each DW days since 1/1/78, DB BCD hour, DB BCD min.
    getpcode()/setpcode(c) fn 108 CP/M 3 and above (not MP/M II); return code,
                                  or 0xFFFF if unsupported. Valid codes: 0..0FEFFh ok,
                                  0FF00h..0FF7Fh fatal, 0FFFDh hw error, 0FFFEh ^C.
    getcmode()/setcmode(m) fn 109 console mode bitmap, same support/return as pcode.
    gettime(stamp)        fn 105  version > 22h; stamp[5] = DW days since 1/1/78,
                                  DB BCD hour, DB BCD min, (DOS Plus: DB BCD sec).
                                  Returns BCD seconds, or -1 if unsupported.
    settime(stamp)        fn 104  same stamp layout (the 4-byte part is the same as in
                                  getftime/setftime); 0 ok, -1 unsupported.
    p_cli(cmd)            fn 150  MP/M and Concurrent CP/M only; 0 ok, 0xFF fail/unsupported.
                                  Auto-detaches the caller and gives the console to the child.
                                  To wait for the child: p_priority(<198), conattach(),
                                  p_priority(200). Priority must be strictly < 198 (not equal)
                                  or conattach() blocks indefinitely on CCP/M 3.1.
    p_priority(prio)      fn 145  MP/M, Concurrent CP/M only; 0 = highest, 255 = lowest,
                                  transients start at 200; 0 ok, -1 unsupported.
    conattach()           fn 146  MP/M, Concurrent CP/M only; blocks until the default
                                  console is free; 0 ok, -1 unsupported.
    condetach()           fn 147  MP/M, Concurrent CP/M and CP/M-86 v4 (>= 41h); runs in
                                  the background; 0 ok, -1 unsupported.
    getpid()              fn 156  MP/M, Concurrent CP/M only; PD address of this process
                                  (offset in the system data segment), 0xFFFF unsupported.
    getppid()             fn 156  PD address of the parent process, 0 = none, 0xFFFF unsupported.
    getpriority()         fn 156  priority (0..255) of this process, -1 unsupported.
    getppriority()        fn 156  priority (0..255) of the parent, -1 = no parent/unsupported.
  p_chain (fn 47) is in sub.h.
*/

#define fcb_is_sys(fcb) (fcb[10]&~0x7F)
#define fcb_is_ro(fcb) (fcb[9]&~0x7F)
#define fcb_set_sys(fcb) (fcb[10]|=0x80)
#define fcb_set_ro(fcb) (fcb[9]|=0x80)
#define fcb_clear_sys(fcb) (fcb[10]&=0x7F)
#define fcb_clear_ro(fcb) (fcb[9]&=0x7F)


#endif
