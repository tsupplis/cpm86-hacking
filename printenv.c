/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include <util.h>
#include <dirent.h>

#ifndef __STDC__
int main(argc,argv)
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
    int i;

    i=1;
    while(i<argc) {
        if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"-H")) {
            fprintf(stderr,"INF: Usage: printenv [-h]\n");
            fprintf(stderr,"INF: Display environment variables \n");
            fprintf(stderr,"INF:     -h for help\n");
            exit(0);
        }
        if(argv[i][0]=='-') {
            int j=1;
            while(argv[i][j]) {
                switch(argv[i][j]) {  
                default:
                    fprintf(stderr,"INF: Usage: printenv [-h]\n");
                    fprintf(stderr,"ERR: Wrong parameters\n");
                    exit(0);
                    break;
                }
                j++;
            }
            i++;
            continue;
        }
        break;
    }
#ifdef DEBUG
    fprintf(stderr,"INF: OS var: %x, OS type: %x, current drive: %d, CCP drive: %d, current user: %d\n",

        osver(),
        ostype(),
        getcurdrv(),
        getccpdrv(),
        getusr());
#endif
    printenv(stdout);
    return 0;
}
