/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <os.h>

static unsigned char scb_pb[255+2];

int ccpm_sysvar(p)
    int p;
{
    int es;
    int bx;
    int ax;
    int i;

    memset(scb_pb,0,sizeof(scb_pb));
    scb_pb[0]=p;
    ax=bdosx(49,scb_pb,&es,&bx);
    fprintf(stderr,"%04x\n",bx);
    for(i=0;i<32;i++) {
        fprintf(stderr,"%02x ",scb_pb[i]);
        if(!((i+1)%8)) {
            fprintf(stderr,"\n");
        }
    }
    fprintf(stderr,"\n");
}

int getcppdrive() {
    if(osver()>0x22) {
    } else {
    }
}

#ifndef __STDC__
int main(argc, argv) 
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
    int es;
    int bx;
    int ax;
    int i;

    if(osver()>0x22) {
        ccpm_sysvar(0);
        ccpm_sysvar(5);
        ccpm_sysvar(0x86);
    } else {
        ax=bdosx(49,0,&es,&bx);
        for(i=0;i<100;i++) {
            fprintf(stderr,"%02x ",peek(bx+i,es));
            if(!((i+1)%8)) {
                fprintf(stderr,"\n");
            }
        }
    }
    fprintf(stderr,"\n");
    fprintf(stderr,"curdrv %u\n",getcurdrv());
    fprintf(stderr,"ccpdrv %u\n",getccpdrv());
    fprintf(stderr,"\x1b[=4h\n");
    fprintf(stderr,"\x1b[=0h\n");
    fprintf(stderr,"\x1b[=3h\n");
    exit(0);
}

