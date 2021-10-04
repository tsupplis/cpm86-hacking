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


#ifndef __STDC__
int keyb();
int osver();
int bdosx();
delay();
#else
int keyb();
int osver();
int bdosx(int cx, int ax, int* es, int *bx);
void delay(unsigned int delay);
#endif

#ifndef __STDC__
dpb_load();
#else
dpb_load(int drive, dpb_t*dpb);
#endif

#endif
