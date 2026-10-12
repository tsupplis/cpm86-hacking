#ifndef _OS_H_
#define _OS_H_

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

#define fcb_is_sys(fcb) (fcb[10]&~0x7F)
#define fcb_is_ro(fcb) (fcb[9]&~0x7F)
#define fcb_set_sys(fcb) (fcb[10]|=0x80)
#define fcb_set_ro(fcb) (fcb[9]|=0x80)
#define fcb_clear_sys(fcb) (fcb[10]&=0x7F)
#define fcb_clear_ro(fcb) (fcb[9]&=0x7F)


#endif
