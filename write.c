/*
     The contents of this file are subject to the Mozilla Public License
     Version 1.0 (the "License"); you may not use this file except in
     compliance with the License. You may obtain a copy of the License at
     http://www.mozilla.org/MPL/

     Software distributed under the License is distributed on an "AS IS"
     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
     License for the specific language governing rights and limitations
     under the License.
*/

#include <stdio.h>

#define BUFLEN (2048)
#define GETCH_BUFLEN (12)

static char getch_buffer[GETCH_BUFLEN];

int getch()
{
    int i,c,d;
    static int s=0;
    static int o=0;

    if(s>0) {
        c=getch_buffer[o];s--;o++;
        o=o%GETCH_BUFLEN;
        return c;
    }
    while(!(c=bdos(6,255))) 
        continue;
    while(s<GETCH_BUFLEN && (d=bdos(6,255))) {
        if(c!=27) { 
            getch_buffer[(o+s)%GETCH_BUFLEN]=d;
            s++;
        }
    }
    return c;
}

char buffer[BUFLEN+1];

#ifdef __LEGACY__
int main(argc, argv) 
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
    int c;
    int offset=0;
    char * flag_access="w";
    int arg_offset=1;
    FILE * fp=stdout;
    int eof=0;

    freopen("con:", "r", stdin);
    freopen("con:", "w", stdout);
    if(argc<2) {
        fprintf(stderr,"INF: Usage: write -h | [-a] filename\n");
        fprintf(stderr,"ERR: Wrong parameters\n");
        return 1;
    }
    if(!strcmp("-h",argv[arg_offset]) || !strcmp("-H",argv[arg_offset])) {
        fprintf(stderr,"INF: Usage: write -h | [-a] filename\n");
        fprintf(stderr,"INF: where filename is [user/]filespec\n");
        fprintf(stderr,"INF:     -h for help\n");
        fprintf(stderr,"INF:     -a to happen to existing file\n");
        return 0;
    }
    if(!strcmp("-a",argv[arg_offset]) || !strcmp("-A",argv[arg_offset])) {
        flag_access="a";
        arg_offset=2;
        if(argc<3) {
            fprintf(stderr,"INF: Usage: write -h | [-a] filename\n");
            fprintf(stderr,"ERR: Wrong parameters\n");
            return 1;
        }
    }
    fp=fopen(argv[arg_offset],flag_access);
    if(!fp) {
        fprintf(stderr,"ERR: Cannot open '%s'",argv[arg_offset]);
        return 1;
    }
    fprintf(stderr,"INF: Opening file '%s' in %s mode ...\n",argv[arg_offset],
            flag_access[0]=='a'?"append":"overwrite");
    fprintf(stderr,"INF: --BEGIN--\n");
    while(1) {
        c=getch();
        if(c=='\b') {
            if(offset>0) {
                printf("\b \b");
                offset--;
            }
            continue;
        }
        switch(c) {
            case EOF:
                printf("%s","<EOF>\n");
                eof=1;
                break;
            case 3:
                printf("%s","^C\n");
                eof=1;
                break;
            case 26:
                printf("%s","^Z\n");
                eof=1;
                break;
            case '\r':
                printf("%s","\r\n");
                buffer[offset]=0;
                fprintf(fp,"%s\n",buffer);
                offset=0;
                continue;
            default:
                if(c<' ') {
                    continue;
                }
        }
        if(eof) {
            break;
        }
        if(offset==BUFLEN) {
            continue;
        }
        buffer[offset++]=c;
        putchar(c);
    }
    if(offset>0) {
        buffer[offset]=0;
        fprintf(fp,"%s",buffer);
        fflush(fp);
    }
    fputc(26,fp);
    fprintf(stderr,"INF: --END--\n");
    fprintf(stderr,"INF: Closing file '%s' ...",argv[arg_offset]);
    fflush(fp);
    fclose(fp);
    return 0;
}
