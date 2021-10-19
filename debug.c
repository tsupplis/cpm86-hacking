/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include "debug.h"

/*#define DEBUG*/
#ifndef __STDC__
unsigned long debug_dump_hex(fd, data, size, offset, raw) 
    FILE *fd;
    char *data;
    unsigned int size;
    unsigned long offset;
    int raw;
#else
unsigned long debug_dump_hex(FILE *fd, char*data, unsigned int size,
    unsigned long offset, int raw)
#endif
{
#ifdef DEBUG
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
#endif
    return offset;
}

