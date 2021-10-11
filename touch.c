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
    FILE * fp; 
    char * filename=0;

    i=1;
    while(i<argc) {
        if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"-H")) {
            fprintf(stderr,"INF: Usage: touch [-h] | filespec\n");
            fprintf(stderr,"INF: Empty file creation utility \n");
            fprintf(stderr,"INF:     -h for help\n");
            exit(0);
        }
        if(argv[i][0]=='-') {
            int j=1;
            while(argv[i][j]) {
                switch(argv[i][j]) {  
                default:
                    fprintf(stderr,"INF: Usage: touch [-h] | [-a][-i][-f] filespec\n");
                    fprintf(stderr,"ERR: Wrong parameters\n");
                    exit(0);
                    break;
                }
                j++;
            }
            i++;
            continue;
        }
        filename=argv[i];
        break;
    }
 

    if(!filename) {
        fprintf(stderr,"INF: Usage: touch [-h] | filespec\n");
        fprintf(stderr,"ERR: Wrong parameters (no filespec)\n");
        exit(-1);
    }
    fp=fopen(filename,"a");
    if(fp) {
        fclose(fp);
    } else {
        fprintf(stderr,"ERR: Failed to create %s\n",filename);
        exit(-1);
    }
    return 0;
}
