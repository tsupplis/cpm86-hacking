/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include <util.h>
#include <dirent.h>

int getch() 
{
    int i, c, d;
    static int s = 0;

    if (s > 0) {
        c = s;
        s = 0;
        return c;
    }
    while (!(c = bdos(6, 255)))
        ;
    while (d = bdos(6, 255))
        ;
    return c;
}


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
    int ask=0;
    int all=0;
    int force=0;
    FILE *conout;

    i=1;
    while(i<argc) {
        if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"-H")) {
            fprintf(stderr,"INF: Usage: rm [-h] | [-a][-i][-f] filepat\n");
            fprintf(stderr,"INF: File delete utility \n");
            fprintf(stderr,"INF: where filepat is [user/]filespec with wildcard\n");
            fprintf(stderr,"INF:     -h for help\n");
            fprintf(stderr,"INF:     -a to include both dir and sys files \n");
            fprintf(stderr,"INF:     -i for user validation per file \n");
            fprintf(stderr,"INF:     -f force file deletion even if read only \n");
            exit(0);
        }
        if(argv[i][0]=='-') {
            int j=1;
            while(argv[i][j]) {
                switch(argv[i][j]) {  
                case 'i':
                case 'I':
                    ask=1;
                    break;
                case 'f':
                case 'F':
                    force=1;
                    break;
                case 'a':
                case 'A':
                    all=1;
                    break;
                default:
                    fprintf(stderr,"INF: Usage: rm [-h] | [-a][-i][-f] filespec\n");
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
        fprintf(stderr,"INF: Usage: rm [-h] | [-a][-i][-f] filepat\n");
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
 
    if(ask) {
        freopen("con:", "r", stdin);
        freopen("con:", "w", stdout);
        conout=stdout;
    } else {
        conout=stderr;
    }
    cursor=root;
    while(cursor) {
        int offset=0;
        int j=0;
        int dot=0;
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
        if(dirent_is_ro(cursor)) {
            if(force) {
                char ro_fcb[40];
                int ou;
                int u;
                memset(ro_fcb,0,sizeof(ro_fcb));
                ou=getusr();
                u=dirent_fcb(ro_fcb,cursor);
                fprintf(stderr,"DBG %d\n",u);
                fcb_clear_ro(ro_fcb);
                setusr(u);
                debug_dump_hex(stderr, (unsigned char*)(ro_fcb), 33, 0L, 0);
                bdos(30,ro_fcb);
                setusr(ou);;
            } else {
                cursor=cursor->next;
                continue;
            }
        } 
        {
            int delete=1;
            fprintf(conout,"INF: Deleting %s ",path);
            if(ask) {
                int c;
                
                fprintf(conout,"(Y/N)");
                while(1) {
                    c=getch();
                    if(c=='y'||c=='Y') {
                        fprintf(stdout,"\x08\x08\x08\x08\x08     ");
                        fprintf(stdout,"\x08\x08\x08\x08\x08");
                        delete=1;
                        break;
                    }
                    if(c=='n'||c=='N') {
                        fprintf(stdout,"\x08\x08\x08\x08\x08Skipping\n");
                        delete=0;
                        break;
                    }
                    if(c==3) {
                        fprintf(stdout,"\x08\x08\x08\x08\x08     ");
                        fprintf(stdout,"\x08\x08\x08\x08\x08^C");
                        exit(0);
                        break;
                    }
                }
            } else {
            }
            if(delete) {
                if(unlink(path)) {
                    fprintf(conout,"Failed\n",path);
                } else {
                    fprintf(conout,"OK\n",path);
                }
            }
        }
        cursor=cursor->next;
    }
    exit(0);
}
