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
int main(argc, argv) 
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
    int c, d, ctr = 0;
    int process = 1;
    FILE *infp = 0;
    int flag_pause=1;
    char * infile=0;
    int i;

    i=1;
    while (i < argc) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-H")) {
            fprintf(stderr, "INF: Usage: more -h | [infile]\n");
            fprintf(stderr, "INF: where infile is [user/]filespec\n");
            fprintf(stderr, "INF:     -h for help\n");
            exit(0);
        }

        if (!infile) {
            infile = argv[i];
            i++;
            break;
        }
    }


    if(infile) {
        infp = fopen(infile, "r");
        if (0 == infp) {
            fprintf(stderr, "ERR: File %s not found or accessible\n", argv[1]);
            exit(-1);
        }
    } else {
        infp=stdin;
    }

    if(!infp && isatty(0)) {
        fprintf(stderr, "ERR: No file specified and tty not redirected\n");
        exit(-1);
    }
    if(!isatty(fileno(stdout))) {
        flag_pause=0;
    }


    while (process) {
        if (infile)
            c = fgetc(infp);
        else
            c = getchar();
        if (c == EOF || c == 26)
            break;

        putchar(c);
        if (c == '\n') {
            ctr++;
            if (flag_pause) {
                if (ctr > 22) {
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
            } else {
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
        }
    }
    if (infile) {
        fclose(infp);
    }
    exit(0);
}