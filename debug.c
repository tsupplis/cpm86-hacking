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
#ifndef __LEGACY__
#include <stdlib.h>
#endif
#include "debug.h"

#ifdef __LEGACY__
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

