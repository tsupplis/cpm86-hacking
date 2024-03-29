/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#else
char *malloc();
#endif
#include <util.h>

#ifndef __STDC__
dpb_load(drive, dpb)
    int drive; 
    dpb_t* dpb; 
#else
dpb_load(int drive, dpb_t*dpb) 
#endif
{
    int i;
    int es;
    int bx;
    int ax;
    int segs[4];
    int curdrive=getcurdrv();
    unsigned char *ptr=(unsigned char *)dpb;

    setcurdrv(drive);
    segread(segs);
    ax=bdosx(31,0,&es,&bx);
    for(i=0;i<15;i++) 
        ptr[i]=peekb(bx+i,es);
    setcurdrv(curdrive);
}


