/*
 * cmdseg.c -- bounded, sequential access to loaded CMD groups.
 * Licensed under the MIT license. See LICENSE in the project root.
 */
#include <stdio.h>
#include "cmdseg.h"

#ifdef CMDSEG_TEST
/* Link-time substitutes for host tests; not part of the public API. */
extern int cs_info(unsigned group, unsigned *segment, unsigned long *size);
extern int cs_copy(unsigned segment, unsigned offset, void *buffer,
                   unsigned count, int writing);
#else
#include <regs.h>
#include <x86.h>

static int cs_info(unsigned group, unsigned *segment, unsigned long *size)
{
    struct SREGS regs;
    unsigned offset;
    segread(&regs);
    offset = (group - 1) * 6;
    *segment = peekw(offset + 3, regs.ds);
    *size = (unsigned long)(unsigned)peekw(offset, regs.ds) +
            ((unsigned long)(peekb(offset + 2, regs.ds) & 255) << 16) + 1;
    return 0;
}

static int cs_copy(unsigned segment, unsigned offset, void *buffer,
                   unsigned count, int writing)
{
    struct SREGS regs;
    unsigned address;
    address = (unsigned)buffer;
    if ((unsigned long)count > 65536UL - address) return CMDSEG_RANGE;
    segread(&regs);
    /* movblock length zero means 64 KiB, so never call it for an empty copy. */
    if (count) {
        if (writing) movblock(address, regs.ds, offset, segment, count);
        else movblock(offset, segment, address, regs.ds, count);
    }
    return 0;
}
#endif

void cmdseg_close(cmdseg *h)
{
    if (!h) return;
    h->_segment = h->_mode = 0;
    h->_start = h->_size = h->_pos = 0;
}

int cmdseg_open(cmdseg *h, unsigned group, unsigned mode)
{
    unsigned segment;
    unsigned long size;
    int result;
    if (!h) return CMDSEG_ARG;
    cmdseg_close(h);
    if ((group != CMD_EXTRA && (group < CMD_AUX1 || group > CMD_AUX4)) ||
        (mode != CMD_RDONLY && mode != CMD_RDWR)) return CMDSEG_ARG;
    result = cs_info(group, &segment, &size);
    if (result < 0) return result;
    if (!segment || !size) return CMDSEG_MISSING;
    if (size > 65536UL) return CMDSEG_LARGE;
    /* Reject a view whose physical end would wrap the 8086 address space. */
    if ((unsigned long)segment * 16 + size > 1048576UL) return CMDSEG_RANGE;
    h->_segment = segment;
    h->_mode = mode;
    h->_size = size;
    return 0;
}

int cmdseg_slice(cmdseg *h, const cmdseg *parent,
                 unsigned long offset, unsigned long length)
{
    cmdseg view;
    if (!h || !parent) return CMDSEG_ARG;
    if (!parent->_segment) return CMDSEG_CLOSED;
    if (offset > parent->_size || length > parent->_size - offset)
        return CMDSEG_RANGE;
    view = *parent;
    view._start += offset;
    view._size = length;
    view._pos = 0;
    *h = view;
    return 0;
}

unsigned long cmdseg_size(const cmdseg *h)
{
    return h && h->_segment ? h->_size : 0;
}

unsigned long cmdseg_tell(const cmdseg *h)
{
    return h && h->_segment ? h->_pos : 0;
}

int cmdseg_seek(cmdseg *h, unsigned long offset)
{
    if (!h) return CMDSEG_ARG;
    if (!h->_segment) return CMDSEG_CLOSED;
    if (offset > h->_size) return CMDSEG_RANGE;
    h->_pos = offset;
    return 0;
}

static long transfer(cmdseg *h, void *buffer, unsigned count, int writing)
{
    unsigned long remaining;
    int result;
    if (!h) return CMDSEG_ARG;
    if (!h->_segment) return CMDSEG_CLOSED;
    if (writing && h->_mode != CMD_RDWR) return CMDSEG_RDONLY;
    if (count && !buffer) return CMDSEG_ARG;
    /* Also enforce the 16-bit count contract in hosted builds. */
    if ((unsigned long)count > 65535UL) return CMDSEG_RANGE;
    remaining = h->_size - h->_pos;
    if ((unsigned long)count > remaining) {
        if (writing) return CMDSEG_RANGE;
        count = (unsigned)remaining;
    }
    if (!count) return 0;
    result = cs_copy(h->_segment, (unsigned)(h->_start + h->_pos),
                     buffer, count, writing);
    if (result < 0) return result;
    h->_pos += count;
    return (long)count;
}

long cmdseg_read(cmdseg *h, void *buffer, unsigned count)
{
    return transfer(h, buffer, count, 0);
}

long cmdseg_write(cmdseg *h, const void *buffer, unsigned count)
{
    return transfer(h, (void *)buffer, count, 1);
}

int cmdseg_getc(cmdseg *h)
{
    unsigned char byte;
    long result;
    result = cmdseg_read(h, &byte, 1);
    if (result < 0) return (int)result;
    return result ? (int)byte : CMDSEG_EOF;
}
