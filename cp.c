/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
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
    dirent_t * root;
    dirent_t * cursor;
    int rc; 
    char * pat=0;

    i=1;
    while(i<argc) {
        if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"-H")) {
            fprintf(stderr,"INF: Usage: cp [-h] | sourcespec targetspec\n");
            fprintf(stderr,"INF: File copy utility\n");
            fprintf(stderr,"INF: where sourcespec is [user/]filespec with wildcard\n");
            fprintf(stderr,"INF: where targetspec is [user/]filespec if one source file\n");
            fprintf(stderr,"INF:                  or [user/]drivespec if multiple source files\n");
            fprintf(stderr,"INF:     -h for help\n");
            exit(0);
        }
        if(argv[i][0]=='-') {
            int j=1;
            while(argv[i][j]) {
                switch(argv[i][j]) {  
                default:
                    fprintf(stderr,"INF: Usage: cp [-h] | sourcespec targetspec\n");
                    fprintf(stderr,"ERR: Wrong parameters\n");
                    exit(0);
                    break;
                }
                j++;
            }
            i++;
            continue;
        }
        pat=argv[i];
        break;
    }
 
    if(!pat) {
        fprintf(stderr,"INF: Usage: cp [-h] | sourcespec targetspec\n");
        fprintf(stderr,"ERR: Wrong parameters (no filespec)\n");
        exit(-1);
    }
    rc=dirent_load(pat,&root,(int*)0,(int*)0,0,0);
    if(rc) {
        if(rc=255) {
            fprintf(stderr,"INF: No file found\n");
            exit(0);
        } else {
            fprintf(stderr,"ERR: Internal memory error\n");
            exit(-1);
        }
    } 
    exit(0);
}
