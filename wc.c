/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/


#include <stdio.h>
#include <ctype.h>
#include <dirent.h>

#ifdef __STDC__
#include <stdlib.h>
#else
char *malloc();
#endif

#define BUFLEN (16*1024)

#ifndef __STDC__
int parse_file(filename, w, l, c) 
    char *filename;
    unsigned long * w;
    unsigned long * l;
    unsigned long * c;
#else
int parse_file(char * filename, unsigned long *w, unsigned long *l, 
    unsigned long *c)
#endif
{
    FILE *infp;
    int count; 
    int ic;
    int state=0;
    int eof=0;
    char * buffer;

    if (!(infp = fopen(filename, "r"))) {
        return -1;
    }
    buffer=malloc(BUFLEN);

    *w = 0;
    *l = 0;
    *c = 0;
    while(!eof && (count = fread(buffer,1,BUFLEN,infp)) != 0) {
        int i;
        for(i=0;i<count;i++) {
            ic=buffer[i];
            if(ic==26) {
                eof=1;
                break;
            }
            if(ic == '\r') {
                l[0]++;
            }
            if(isalnum(ic)) {
                if (state != 1) {
                    state = 1;
                    w[0]++;
                }
            } else {
                state = 0;
            }
            c[0]++;
        }
    }
    fclose(infp);
    free(buffer);
    return 0;
}

#ifndef __STDC__
int main(argc, argv) 
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
    int i;
    int all=0;
    unsigned long tw = 0;
    unsigned long tl = 0; 
    unsigned long tc = 0;
    unsigned long w = 0;
    unsigned long l = 0; 
    unsigned long c = 0;
    int flag_break=1;
    int ctrlc=0;
    dirent_t * root=0;

    i=1;
    while(i<argc) {
        if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"-H")) {
            fprintf(stderr,"INF: Usage: wc [-h] | [-a] filepat [filepat] ...\n");
            fprintf(stderr,"INF: File word/line/character count utility \n");
            fprintf(stderr,"INF: where filepat is [user/]filespec with wildcard\n");
            fprintf(stderr,"INF:     -h for help\n");
            fprintf(stderr,"INF:     -a to include both dir and sys files \n");
            exit(0);
        }
        if(argv[i][0]=='-') {
            int j=1;
            while(argv[i][j]) {
                switch(argv[i][j]) {  
                case 'a':
                case 'A':
                    all=1;
                    break;
                default:
                    fprintf(stderr,
                        "INF: Usage: wc [-h] | [-a] filepat [filepat] ...\n");
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
    if(!isatty(fileno(stdout))) {
        flag_break=0;
    }

    while(i<argc && !ctrlc) {
        dirent_t * cursor;
        int rc=dirent_load(argv[i],&root,(int*)0,(int*)0,0,0);
        rc=dirent_load(argv[i],&root,(int*)0,(int*)0,0,0);
        rc=dirent_load(argv[i],&root,(int*)0,(int*)0,0,0);
        if(rc) {
            if(rc==255) {
                fprintf(stderr,"INF: No file found for '%s'\n",argv[i]);
                i++;
                continue;
            } else {
                fprintf(stderr,"ERR: Internal memory error\n");
                exit(-1);
            }
        } 
        cursor=root;
        while(cursor) {
            int offset=0;
            int dot=0;
            int j;

            char path[20+1];
            sprintf(path,"%d/%c:",(int)(cursor->entry[0]),cursor->drive+'A');
            offset=strlen(path);
            for(j=1;j<9;j++) {
                if(((cursor->entry[j])&0x7F)==' ') {
                    continue;
                }
                path[offset++]=cursor->entry[j]&0x7F;
            }
            for(j=9;j<12;j++) {
                if(((cursor->entry[j])&0x7F)==' ') {
                    break;
                }
                if(!dot) {
                    dot=1;
                    path[offset++]='.';
                }
                path[offset++]=cursor->entry[j]&0x7F;
            }
            path[offset]=0; 
            
            if(dirent_is_sys(cursor) && !all) {
                cursor=cursor->next;
                continue;
            }
            if (!flag_break) {
                int d;
                while(bdos(11,0)) {
                    d=bdos(1,0);
                    if(d==3) ctrlc=1;
                }
            }
            if(parse_file(path,&w,&l,&c)) {
                fprintf(stderr,"ERR: File error on '%s'\n",path);
            } else {
                tw+=w;
                tl+=l;
                tc+=c;
                fprintf(stdout, "%-20s %-6lu %-6lu %-6lu\n",path,w,l,c);
            }
            cursor=cursor->next;
            if(ctrlc) {
                break;
            }
        }
        dirent_free(root);
        i++;
    }
    if(!ctrlc) {
        fprintf(stdout, "%-20s %-6lu %-6lu %-6lu\n","TOTAL",tw,tl,tc);
    }
    exit(0);
}
