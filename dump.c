/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#include <sgtty.h>


#ifdef __LEGACY__
unsigned long dump_hex(fd, data, size, offset, raw) 
    FILE *fd;
    char *data;
    unsigned int size;
    unsigned long offset;
    int raw;
#else
unsigned long dump_hex(FILE *fd, char*data, unsigned int size,
    unsigned long offset, int raw)
#endif
{
    char ascii[17];
    unsigned int i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        if (i % 16 == 0 && !raw) {
            fprintf(fd, "%06x: ", offset);
        }
        offset++;
        fprintf(fd, "%02x ", ((unsigned char *)data)[i]);
        if (data[i] >= ' ' && data[i] <= '~') {
            ascii[i % 16] = data[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i + 1) % 8 == 0 || i + 1 == size) {
            if (!raw) {
                fprintf(fd, " ");
            }
            if ((i + 1) % 16 == 0) {
                if (raw) {
                    fprintf(fd, "\n");
                } else {
                    fprintf(fd, "|  %s \n", ascii);
                }
            } else if (i + 1 == size) {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8) {
                    if (!raw) {
                        fprintf(fd, " ");
                    }
                }
                for (j = (i + 1) % 16; j < 16; ++j) {
                    if (!raw) {
                        fprintf(fd, "   ");
                    }
                }
                if (raw) {
                    fprintf(fd, "\n");
                } else {
                    fprintf(fd, "|  %s \n", ascii);
                }
            }
        }
    }
    return offset;
}

int getch() {
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

#define BUFLEN 1024
char buffer[BUFLEN];

#ifdef __LEGACY__
int main(argc, argv) 
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
    int c, i, ctr = 0;
    int flag_pause = 0;
    int flag_raw = 0;
    unsigned long offset = 0;
    char *infile = 0;
    FILE *infp = 0;
    int process = 1;


    i = 1;
    while (i < argc) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-H")) {
            fprintf(stderr, "INF: Usage: dump -h | [-p][-r] [infile]\n");
            fprintf(stderr, "INF: where infile is [user/]filespec\n");
            fprintf(stderr, "INF:     -h for help\n");
            fprintf(stderr, "INF:     -p pausing every page\n");
            fprintf(stderr, "INF:     -r raw output\n");
            exit(0);
        }
        if(argv[i][0]=='-') {
            int j=1;
            while(argv[i][j]) {
                switch(argv[i][j]) {  
                case 'r':
                case 'R':
                    flag_raw=1;
                    break;
                case 'p':
                case 'P':
                    flag_pause=1;
                    break;
                default:
                    fprintf(stderr, "INF: Usage: dump -h | [-p][-r] [infile]\n");
                    fprintf(stderr,"ERR: Wrong parameters\n");
                    exit(0);
                    break;
                }
                j++;
            }
            i++;
            continue;
        }
 
        if (!infile) {
            infile = argv[i];
            i++;
            break;
        }
    }

    if(!isatty(fileno(stdout))) {
        flag_pause=0;
    }
    if (!infile) {
        if(!isatty(fileno(stdin))) {
            infp = stdin;
        } else {
            fprintf(stderr, "ERR: No file specified and tty not redirected\n");
            exit(-1);
        }
    } else {
        infp = fopen(infile, "r");
        if (!infp) {
            fprintf(stderr, "ERR: File %s not found or accessible\n", argv[1]);
            exit(-1);
        }
        fclose(stdin);
    }

    while (process) {
        c = fread(buffer, 1, flag_pause ? 16 : BUFLEN, infp);
        if (c < 1) {
            break;
        }
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

        offset = dump_hex(stdout, buffer, c, offset, flag_raw);
        ctr++;
        if (ctr > 22) {
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
    }
    if (infile) {
        fclose(infp);
    }
    exit(0);
}
