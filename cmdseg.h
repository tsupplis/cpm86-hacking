#ifndef _CMDSEG_H_
#define _CMDSEG_H_

/* Licensed under the MIT license. See LICENSE in the project root. */

/* cmdseg.c -- bounded views of loader-owned CMD EXTRA/AUX groups. */
#define CMD_EXTRA   3
#define CMD_AUX1    5
#define CMD_AUX2    6
#define CMD_AUX3    7
#define CMD_AUX4    8
#define CMD_RDONLY  0
#define CMD_RDWR    1

#define CMDSEG_EOF      (-1)
#define CMDSEG_CLOSED   (-2)
#define CMDSEG_ARG      (-3)
#define CMDSEG_RANGE    (-4)
#define CMDSEG_RDONLY   (-5)
#define CMDSEG_MISSING  (-6)
#define CMDSEG_LARGE    (-7)

/* Caller-owned handle. Fields are private; use the functions below. */
typedef struct cmdseg {
    unsigned int _segment, _mode;
    unsigned long _start, _size, _pos;
} cmdseg;

#ifndef __STDC__
int cmdseg_open();
int cmdseg_slice();
unsigned long cmdseg_size();
unsigned long cmdseg_tell();
int cmdseg_seek();
long cmdseg_read();
long cmdseg_write();
int cmdseg_getc();
void cmdseg_close();
#else
int cmdseg_open(cmdseg *h, unsigned int group, unsigned int mode);
int cmdseg_slice(cmdseg *h, const cmdseg *parent,
                 unsigned long offset, unsigned long length);
unsigned long cmdseg_size(const cmdseg *h);
unsigned long cmdseg_tell(const cmdseg *h);
int cmdseg_seek(cmdseg *h, unsigned long offset);
long cmdseg_read(cmdseg *h, void *buffer, unsigned int count);
long cmdseg_write(cmdseg *h, const void *buffer, unsigned int count);
int cmdseg_getc(cmdseg *h);
void cmdseg_close(cmdseg *h);
#endif

#endif
