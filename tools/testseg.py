#!/usr/bin/env python3
"""Local CMD segment tests: host bounds and real Aztec code in emu2 (no PCE)."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent.parent
HOST = r'''
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "cmdseg.h"
static unsigned char memory[65536];
static unsigned seg = 0x2000;
static unsigned long capacity = 65536UL;
static unsigned copies;
static int fail;
int cs_info(unsigned g, unsigned *s, unsigned long *n) {
    assert(g == CMD_EXTRA || (g >= CMD_AUX1 && g <= CMD_AUX4));
    *s = seg; *n = capacity; return 0;
}
int cs_copy(unsigned s, unsigned o, void *b, unsigned n, int w) {
    assert(s == seg && n && (unsigned long)o + n <= capacity);
    if (fail) return CMDSEG_RANGE;
    copies++;
    if (w) memcpy(memory + o, b, n); else memcpy(b, memory + o, n);
    return 0;
}
int main(void) {
    cmdseg h, a, b;
    unsigned char buf[8];
    unsigned i;
    for (i=0;i<65536;i++) memory[i]=(unsigned char)i;
    assert(cmdseg_open(&h,CMD_EXTRA,CMD_RDONLY)==0);
    assert(cmdseg_size(&h)==65536UL);
    assert(cmdseg_getc(&h)==0 && cmdseg_getc(&h)==1);
    assert(cmdseg_slice(&a,&h,250UL,6UL)==0);
    assert(cmdseg_slice(&b,&a,2UL,4UL)==0);
    assert(cmdseg_tell(&a)==0 && cmdseg_getc(&b)==252);
    assert(cmdseg_read(&a,buf,8)==6 && buf[5]==255);
    assert(cmdseg_getc(&a)==CMDSEG_EOF);
    assert(cmdseg_write(&a,buf,1)==CMDSEG_RDONLY);
    assert(cmdseg_slice(&a,&h,65536UL,0UL)==0);
    assert(cmdseg_getc(&a)==CMDSEG_EOF);
    assert(cmdseg_slice(&a,&h,65537UL,0UL)==CMDSEG_RANGE);
    assert(cmdseg_slice(&a,&h,1UL,~0UL)==CMDSEG_RANGE);
    assert(cmdseg_size(&a)==0);
    assert(cmdseg_seek(&h,65535UL)==0 && cmdseg_getc(&h)==255);
    assert(cmdseg_tell(&h)==65536UL && cmdseg_getc(&h)==CMDSEG_EOF);
    assert(cmdseg_seek(&h,65537UL)==CMDSEG_RANGE);
    cmdseg_close(&h);
    assert(cmdseg_getc(&h)==CMDSEG_CLOSED && cmdseg_getc(&b)==253);
    assert(cmdseg_size(&h)==0 && cmdseg_tell(&h)==0);
    for (i=CMD_AUX1;i<=CMD_AUX4;i++) assert(cmdseg_open(&h,i,CMD_RDWR)==0);
    assert(cmdseg_open(&h,4,CMD_RDWR)==CMDSEG_ARG);
    assert(cmdseg_getc(&h)==CMDSEG_CLOSED);
    assert(cmdseg_open(&h,CMD_EXTRA,2)==CMDSEG_ARG);
    assert(cmdseg_open(&h,CMD_EXTRA,CMD_RDWR)==0);
    assert(cmdseg_slice(&h,&h,65534UL,2UL)==0);
    buf[0]=77;buf[1]=88;
    assert(cmdseg_write(&h,buf,3)==CMDSEG_RANGE && cmdseg_tell(&h)==0);
    fail=1; assert(cmdseg_write(&h,buf,2)==CMDSEG_RANGE && cmdseg_tell(&h)==0); fail=0;
    assert(cmdseg_write(&h,buf,2)==2 && memory[65535]==88);
    i=copies;
    assert(cmdseg_read(&h,buf,0)==0 && cmdseg_write(&h,buf,0)==0 && copies==i);
    assert(cmdseg_seek(&h,0UL)==0 && cmdseg_getc(&h)==77);
    assert(cmdseg_read(&h,NULL,1)==CMDSEG_ARG);
    assert(cmdseg_read(&h,buf,65536U)==CMDSEG_RANGE);
    cmdseg_close(&h);cmdseg_close(&h);cmdseg_close(NULL);
    assert(cmdseg_open(NULL,CMD_EXTRA,CMD_RDONLY)==CMDSEG_ARG);
    assert(cmdseg_slice(NULL,&h,0UL,0UL)==CMDSEG_ARG);
    assert(cmdseg_seek(&h,0UL)==CMDSEG_CLOSED);
    assert(cmdseg_seek(NULL,0UL)==CMDSEG_ARG);
    assert(cmdseg_getc(NULL)==CMDSEG_ARG);
    seg=0; assert(cmdseg_open(&h,CMD_EXTRA,CMD_RDONLY)==CMDSEG_MISSING);
    seg=0x2000;capacity=65537UL;
    assert(cmdseg_open(&h,CMD_EXTRA,CMD_RDONLY)==CMDSEG_LARGE);
    capacity=0;assert(cmdseg_open(&h,CMD_EXTRA,CMD_RDONLY)==CMDSEG_MISSING);
    seg=0xffff;capacity=32;
    assert(cmdseg_open(&h,CMD_EXTRA,CMD_RDONLY)==CMDSEG_RANGE);
    puts("PASS host: bounds, modes, slices, independent cursors, failures and 64 KiB");
    return 0;
}
'''
NATIVE = r'''
#include <stdio.h>
#include <stdlib.h>
#include "cmdseg.h"
static char block[17];
static char large[32768];
static void check(int ok, int line) { if (!ok) { printf("FAIL cmdseg line %d\n",line); exit(1); } }
#define need(x) check((x), __LINE__)
int main(void) {
    cmdseg h,a,b;
    unsigned i;
    need(cmdseg_open(&h,CMD_EXTRA,CMD_RDWR)==0);
    need(cmdseg_size(&h)==65536UL);
    need(cmdseg_read(&h,large,32768U)==32768L);
    need(cmdseg_read(&h,large,32768U)==32768L);
    need(cmdseg_tell(&h)==65536UL);
    need(cmdseg_seek(&h,0UL)==0);
    need(cmdseg_read(&h,block,0)==0 && cmdseg_tell(&h)==0);
    need(cmdseg_getc(&h)=='E');
    need(cmdseg_open(&a,CMD_AUX1,CMD_RDONLY)==0);
    need(cmdseg_getc(&a)=='A');
    need(cmdseg_write(&a,block,1)==CMDSEG_RDONLY);
    need(cmdseg_open(&b,CMD_AUX4,CMD_RDONLY)==CMDSEG_MISSING);
    need(cmdseg_slice(&b,&h,65519UL,17UL)==0);
    for(i=0;i<17;i++) block[i]=(char)(239+i);
    need(cmdseg_write(&b,block,17)==17);
    need(cmdseg_tell(&b)==17 && cmdseg_getc(&b)==CMDSEG_EOF);
    need(cmdseg_write(&b,block,1)==CMDSEG_RANGE);
    need(cmdseg_seek(&b,0UL)==0);
    for(i=0;i<17;i++) block[i]=0;
    need(cmdseg_read(&b,block,17)==17);
    for(i=0;i<17;i++) need((unsigned char)block[i]==239+i);
    need(cmdseg_seek(&h,65535UL)==0 && cmdseg_getc(&h)==255);
    need(cmdseg_tell(&h)==65536UL && cmdseg_getc(&h)==CMDSEG_EOF);
    need(cmdseg_seek(&h,0UL)==0);
    /* A near buffer must not wrap DS, including on reads. */
    need(cmdseg_read(&h,(void *)65535U,2)==CMDSEG_RANGE);
    need(cmdseg_tell(&h)==0 && cmdseg_getc(&h)=='E');
    cmdseg_close(&h);
    need(cmdseg_seek(&b,0UL)==0 && cmdseg_getc(&b)==239);
    need(cmdseg_getc(&h)==CMDSEG_CLOSED);
    puts("PASS native: EXTRA/AUX, far copies, last byte, near bounds and zero count");
    return 0;
}
'''

def run(args, cwd):
    result = subprocess.run([str(x) for x in args], cwd=cwd, capture_output=True, timeout=60)
    if result.returncode:
        raise RuntimeError(f'{args}:\n{result.stdout.decode(errors="replace")}\n{result.stderr.decode(errors="replace")}')
    return result.stdout.decode(errors='replace')


def main():
    with tempfile.TemporaryDirectory(prefix='cmdseg-') as name:
        folder = Path(name)
        (folder/'host.c').write_text(HOST)
        run([os.environ.get('HOST_CC','cc'), '-std=c89', '-Wall', '-Wextra',
             '-DCMDSEG_TEST', '-I'+str(ROOT), ROOT/'cmdseg.c', 'host.c', '-o', 'host'], folder)
        print(run([folder/'host'],folder),end='')
        (folder/'native.c').write_text(NATIVE)
        for name in ('cmdseg.h','cmdseg.c'):
            (folder/name).write_bytes((ROOT/name).read_bytes())
        for source,obj in [('cmdseg.c','cmdseg.o'),('native.c','native.o')]:
            run(['aztec43_acc','-target=cpm86','-I.','-squeeze','-c','-o',obj,source],folder)
        run(['aztec43_acc','-target=cpm86','-o','probe.cmd','native.o','cmdseg.o'],folder)
        (folder/'extra.bin').write_bytes(b'EXTRA')
        (folder/'aux.bin').write_bytes(b'AUX1')
        run(['cmdmod','-t','EXTRA','-s','extra.bin','-n','10000','-m','10000','probe.cmd','2'],folder)
        run(['cmdmod','-t','AUX1','-s','aux.bin','probe.cmd','3'],folder)
        output=run([os.environ.get('EMU2','emu2'),'probe.cmd'],folder)
        if 'PASS native:' not in output or 'FAIL' in output:
            raise RuntimeError(output)
        print(output.replace('\r\n','\n'),end='')

if __name__=='__main__':
    main()
