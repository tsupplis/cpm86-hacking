/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
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
    int c, d = 0;
    int process = 1;
    FILE *infp = 0;
    int flag_pause=1;
    char * infile=0;
    int i;
    int col=0;
    int ctr=0;
    int space=0;

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
        if(space) {
            c=' ';
            space--;
        } else {
            if (infile)
                c = fgetc(infp);
            else
                c = getchar();
            if(c==9) {
                c=' ';space+=3;
            } 
        }
        if (c == EOF || c == 26)
            break;

        if (c == '\n') {
            putchar(c);
            col=0;
            ctr++;
        } else {
            putchar(c);
            col++;
        }
        if(col==80) {
            col=col%80;
            ctr++;
        }
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
