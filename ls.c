/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#ifndef __LEGACY__
#include <stdlib.h>
#endif
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

#ifdef __LEGACY__
dirent_print_name(f, d, pack)
    FILE * f;
    dirent_t * d;
    int pack;
#else 
void dirent_print_name(FILE * f, dirent_t * d, int pack)
#endif
{
    int i;
    for(i=1;i<9;i++) {
        if(((d->entry[i])&0x7F)==' ' && pack) {
            break;
        }
        fputc((d->entry[i])&0x7F,f);
    }
    if(((d->entry[9])&0x7F)==' ') {
        if(!pack) {
            fputc(' ',f);
        }
    } else {
        fputc('.',f);
    }
    for(i=9;i<12;i++) {
        if(((d->entry[i])&0x7F)==' ' && pack) {
            break;
        }
        fputc((d->entry[i])&0x7F,f);
    }
}


#ifdef __LEGACY__
int main(argc,argv)
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
    char *pat="*.*";
    dirent_t * root;
    dirent_t * cursor;
    int rc;
    int sort_order=0;
    int count=0;
    int i;
    int flag_all=0;
    int flag_mode=0;
    int flag_pause=0;
    unsigned int block_size=0;
    unsigned long all_size=0;
    int ctr=0;
    int process=1;
    int user;
    int drive;

    i=1;
    while(i<argc) {
        if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"-H")) {
            fprintf(stderr,"INF: Usage: ls [-h] | [-a] [-p] [-s|-r] [-l|-b] filepat\n");
            fprintf(stderr,"INF: where filepat is [user/]filespec with wildcard\n");
            fprintf(stderr,"INF:     -h for help\n");
            fprintf(stderr,"INF:     -a for all files (including system)\n");
            fprintf(stderr,"INF:     -p for for pausing one screen at a time\n");
            fprintf(stderr,"INF:     -s for alphabetic sorting\n");
            fprintf(stderr,"INF:     -r for reverse alphabetic sorting\n");
            fprintf(stderr,"INF:     -l for long listing\n");
            fprintf(stderr,"INF:     -b for basic listing\n");
            exit(0);
        }
        if(argv[i][0]=='-') {
            int j=1;
            while(argv[i][j]) {
                switch(argv[i][j]) {  
                case 'a':
                case 'A':
                    flag_all=1;
                    break;
                case 's':
                case 'S':
                    sort_order=1;
                    break;
                case 'r':
                case 'R':
                    sort_order=-1;
                    break;
                case 'b':
                case 'B':
                    flag_mode=2;
                    break;
                case 'l':
                case 'L':
                    flag_mode=1;
                    break;
                case 'p':
                case 'P':
                    flag_pause=1;
                    break;
                default:
                    fprintf(stderr,
                        "INF: Usage: ls [-h] | [-a] [-p] [-s|-r] [-l|-b] filepat\n");
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
    if(!isatty(fileno(stdout))) {
        flag_pause=0;
    }
    rc=dirent_load(pat,&root,&user,&drive,sort_order,1);
    if(rc) {
        if(rc=255) {
            if(!flag_mode!=2) {
                fprintf(stdout,"Drive %d%c:\n",user, 'A'+drive-1);
                fprintf(stdout,"%d File(s) %luK",count,all_size);
            }
            exit(0);
        } else {
            fprintf(stderr,"ERR: Internal memory error\n");
            exit(-1);
        }
    } 
    cursor=root;
    {
        dpb_t dpb;
        dpb_load(root->drive,&dpb);
        block_size=128<<dpb.bsh;
    }
    if(flag_mode!=2) {
        fprintf(stdout,"Drive %d%c:\n",user, 'A'+drive-1);
        ctr++;
    }
    while(cursor && process) {
        if (!flag_pause) {
            int d;
            int ctrlc=0;
            while(bdos(11,0)) {
                d=bdos(1,0);
                if(d==3) ctrlc=1;
            }
            if(ctrlc) {
                break;
            }
        }

        if(!dirent_is_sys(cursor) || flag_all) {
            unsigned long size;
            if(dirent_is_sys(cursor) && !flag_all) {
                continue;
            }
            size=cursor->blocks*block_size/1024;
            all_size+=size;
            count++;
            if(flag_mode==1) {
                fprintf(stdout,"%c%c ",
                    dirent_is_sys(cursor)?'S':'-',
                    dirent_is_ro(cursor)?'R':'-');
                dirent_print_name(stdout,cursor,0);
                fprintf(stdout," %4luK %4lu FCB(s)\n",size,cursor->fcbs);
                ctr++;
            } else if(flag_mode==2) {
                dirent_print_name(stdout,cursor,1);
                fprintf(stdout,"\n");
                ctr++;
            } else {
                fprintf(stdout,"%c",dirent_is_sys(cursor)?'*':' ');
                dirent_print_name(stdout,cursor,0);
                fprintf(stdout,"%5luK",size);
                if(count%4!=0) {
                    fprintf(stdout,",");
                }
                if((count%4==0)) {
                    fprintf(stdout,"\n");
                    ctr++;
                }
            }
        }
        if(!flag_mode && count%4!=0 && !cursor->next) {
            fprintf(stdout,"\n");
            ctr++;
        }
        if (ctr > 22 || (ctr >21 && !cursor->next)) {
            ctr = 0;
            if (flag_pause) {
                int d = 0;
                int wt = 1;
                printf("[More]");
                while (wt) {
                    d = getch();
                    switch (d) {
                    case 'q':
                    case 'Q':
                        printf("\x08\x08\x08\x08\x08\x08      ");
                        wt = 0;
                        process = 0;
                        break;
                    case 3:
                        printf("\x08\x08\x08\x08\x08\x08^C    ");
                        process = 0;
                        wt = 0;
                        break;
                    case 13:
                        printf("\x08\x08\x08\x08\x08\x08      ");
                        printf("\x08\x08\x08\x08\x08\x08");
                        wt = 0;
                        ctr = 22;
                        break;
                    case 32:
                        printf("\x08\x08\x08\x08\x08\x08      ");
                        printf("\x08\x08\x08\x08\x08\x08");
                        wt = 0;
                        ctr = 0;
                        break;
                    }
                }
            }
        }
        cursor=cursor->next;
    }
    if(flag_mode<2) {
        fprintf(stdout,"%d File(s) %luK",count,all_size);
    }
    dirent_clear(root);
    return 0;
}

