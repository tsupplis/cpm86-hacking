#include "sub.h"

/*
 * FCB for $$$     .SUB
 * Byte  0   : drive (0 = default)
 * Bytes 1-8 : filename, space-padded
 * Bytes 9-11: extension
 * Byte  12  : extent, 14: s2 (bit 7 = not modified), 15: record count (rc)
 * Bytes 16-31: allocation map
 * Byte 32   : current record (cr), auto-advanced by BDOS 21
 *
 * $$$.SUB is a stack read by the CCP from the last record down, inside
 * extent 0 only, so it holds at most 128 records.
 */
static char _fcb[33] = {
    0,                                  /* drive: default */
    '$','$','$',' ',' ',' ',' ',' ',   /* name: "$$$     " */
    'S','U','B',                        /* ext:  "SUB" */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* bytes 12-30 */
    0                                   /* cr (byte 32) */
};

static char _buf[128]; /* sector work buffer */
static int  _created;  /* 1 = file made by sub_open, 0 = existing file    */
static int  _base;     /* record count before sub_open (rollback point)  */

/* clear everything after the name so open/make start from extent 0 */
static void fcb_reset(void)
{
    int i;
    for (i = 12; i < 33; i++)
        _fcb[i] = 0;
}

int sub_open(int flags)
{
    _created = 0;
    _base    = 0;
    fcb_reset();

    if (flags == SUB_APPEND) {
        /* BDOS 15: open; push on top of the records already there */
        if ((bdos(15, _fcb) & 0xFF) != 0xFF) {
            _base = _fcb[15] & 0xFF;
            _fcb[32] = (char)_base;
            return 0;
        }
        fcb_reset();
    }

    /* BDOS 19: delete (ignore error -- file may not exist) */
    bdos(19, _fcb);
    fcb_reset();

    /* BDOS 22: make (create) */
    if ((bdos(22, _fcb) & 0xFF) == 0xFF)
        return -1;

    _created = 1;
    return 0;
}

int sub_records(void)
{
    int n;

    fcb_reset();
    if ((bdos(15, _fcb) & 0xFF) == 0xFF)
        return 0;
    n = _fcb[15] & 0xFF;    /* record count of extent 0 */
    fcb_reset();
    return n;
}

int sub_append(char *cmd)
{
    int i;
    int len;
    char c;

    /* the CCP only reads extent 0: 128 records */
    if ((_fcb[32] & 0xFF) >= 128 || _fcb[12] != 0)
        return -1;

    /* zero the sector buffer */
    for (i = 0; i < 128; i++)
        _buf[i] = 0;

    /* measure and upcase-copy the command */
    len = 0;
    while (cmd[len] != '\0' && len < 125) {
        c = cmd[len];
        if (c >= 'a' && c <= 'z')
            c = c - 'a' + 'A';
        _buf[1 + len] = c;
        len++;
    }

    /* sector layout: [0]=length, [1..len]=cmd, [len+1]=0x00, [len+2]='$' */
    _buf[0]       = (char)len;
    _buf[1 + len] = 0x00;
    _buf[2 + len] = '$';

    /* BDOS 26: set DMA address to _buf */
    bdos(26, _buf);

    /* BDOS 21: sequential write */
    if ((bdos(21, _fcb) & 0xFF) != 0)
        return -1;

    return 0;
}

int sub_close(void)
{
    /* BDOS 16: close file */
    if ((bdos(16, _fcb) & 0xFF) == 0xFF)
        return -1;

    return 0;
}

int sub_abort(void)
{
    if (_created) {
        bdos(16, _fcb);
        bdos(19, _fcb);
        return 0;
    }

    /* drop what was pushed: same as a CCP pop -- lower rc, clear the
       "not modified" flag so close rewrites the directory entry */
    _fcb[15] = (char)_base;
    _fcb[14] = 0;
    if ((bdos(16, _fcb) & 0xFF) == 0xFF)
        return -1;

    return 0;
}

int sub_delete(void)
{
    /* BDOS 19: delete -- ignore error if file does not exist */
    fcb_reset();
    bdos(19, _fcb);
    return 0;
}
