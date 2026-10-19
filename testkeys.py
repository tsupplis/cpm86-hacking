#!/usr/bin/env python3
"""Test the actual getch function bodies with queued high-bit bytes."""
from pathlib import Path
import os
import subprocess
import tempfile
root=Path(__file__).resolve().parent
for filename in ('conio.c','getch.c'):
    text=(root/filename).read_text()
    start=text.rindex('int getch()')
    first=text.index('{',start); depth=1; end=first+1
    while depth:
        depth += (text[end]=='{')-(text[end]=='}'); end+=1
    body=text[start:end]
    source=r'''
#include <stdio.h>
#include <stdlib.h>
#define GETCH_BUFLEN 64
static char *getch_buffer;
static int pos;
int bdos(int fn,int arg) { static int keys[]={0x41,0xC8,0xD0,0xFF,0}; return keys[pos++]; }
''' + body + r'''
int main(void) {
    if(getch()!=0x41 || getch()!=0xC8 || getch()!=0xD0 || getch()!=0xFF) return 1;
    return 0;
}
'''
    with tempfile.TemporaryDirectory(prefix='conio-keys-') as folder:
        work=Path(folder); (work/'keys.c').write_text(source)
        subprocess.run([os.environ.get('CC_HOST','cc'),'-std=gnu89','-fsanitize=address,undefined',
                        str(work/'keys.c'),'-o',str(work/'keys')],check=True)
        subprocess.run([str(work/'keys')],check=True)
    print('PASS:',filename,'returns identical unsigned values for direct and buffered input')
