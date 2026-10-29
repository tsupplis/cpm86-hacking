#include <stdio.h>
#include <util.h>
int main() {
 char f[36]; int rc;
 fcbinit("B:TIME.EXE",f);
 rc=bdos(15,f);
 printf("OPEN %d\n",rc);
 if(rc!=255) { rc=ms_p_cli(f,"B:TIME.EXE"); printf("LAUNCH %d\n",rc); }
 return 0;
}
