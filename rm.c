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
            fprintf(stderr,"INF: Usage: rm [-h] | filepat\n");
            fprintf(stderr,"INF: where filepat is [user/]filespec with wildcard\n");
            fprintf(stderr,"INF:     -h for help\n");
            exit(0);
        }
        if(argv[i][0]=='-') {
            int j=1;
            while(argv[i][j]) {
                switch(argv[i][j]) {  
                default:
                    fprintf(stderr,"INF: Usage: rm [-h] | filespec\n");
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
        fprintf(stderr,"INF: Usage: rm [-h] | filespec\n");
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
 
    cursor=root;
    while(cursor) {
        int offset=0;
        int j=0;
        int dot=0;
        char path[20+1];
        sprintf(path,"%d/%c:",(int)(cursor->entry[0]),cursor->drive+'A'-1);
        offset=strlen(path);
        for(j=1;j<9;j++) {
            if(((cursor->entry[j])&0x7F)==' ') {
                continue;
            }
            path[offset++]=cursor->entry[j];
        }
        for(j=9;j<12;j++) {
            if(((cursor->entry[j])&0x7F)==' ') {
                break;
            }
            if(!dot) {
                dot=1;
                path[offset++]='.';
            }
            path[offset++]=cursor->entry[j];
        }
        path[offset]=0; 
        
        if(dirent_is_ro(cursor)) {
            fprintf(stderr,"INF: Deleting %s, RO, Skipping\n",path);
        } else {
            fprintf(stderr,"INF: Deleting %s ",path);
            if(unlink(path)) {
                fprintf(stderr," Failed\n",path);
            } else {
                fprintf(stderr," OK\n",path);
            }
        }
        
        cursor=cursor->next;
   }
   exit(0);
}
