/*
 * time.c -- CP/M-86 system clock, file timestamps, and Unix epoch.
 * See util.h.
 */
#include <stdio.h>
#include "util.h"

static int month_days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* Copy the 17 bytes at sysdata+20h into tbuf (referenced as tbuf_). */
static char tbuf[18];
static void getsysvartime()
{
    int dummy;			/* forces a bp frame */

#asm
    push es
    push si
    push di
    push bx
    push cx
    mov  cx,31h			; Get Sysdata Address
    int  0E0h			; ES:BX -> sysdata
    add  bx,20h			; ES:BX -> date/time string
    mov  si,bx
    lea  di,tbuf_
    mov  cx,17
gs_cpy:
    mov  al,es:[si]
    mov  [di],al
    inc  si
    inc  di
    dec  cx
    jnz  gs_cpy
    pop  cx
    pop  bx
    pop  di
    pop  si
    pop  es
#endasm
    tbuf[17] = 0;
}

/* Write 17 bytes from tbuf back to sysdata+20h with interrupts disabled. */
static void setsysvartime()
{
    int dummy;			/* forces a bp frame */

#asm
    push es
    push si
    push di
    push bx
    push cx
    cli
    mov  cx,31h			; Get Sysdata Address
    int  0E0h			; ES:BX -> sysdata
    add  bx,20h			; ES:BX -> date/time string
    mov  di,bx
    lea  si,tbuf_
    mov  cx,17
ss_cpy:
    mov  al,[si]
    mov  es:[di],al
    inc  si
    inc  di
    dec  cx
    jnz  ss_cpy
    sti
    pop  cx
    pop  bx
    pop  di
    pop  si
    pop  es
#endasm
}

static int isleap(y)
    int y;
{
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

static int bcd(b)
    int b;
{
    b &= 0xff;
    return (b >> 4) * 10 + (b & 15);
}

static int tobcd(n)
    int n;
{
    return ((n / 10) << 4) | (n % 10);
}

/*
 * days_since_1978: days from 1978-01-01 to year/mon/day (1-based).
 * Returns 0 on error.
 */
static unsigned days_since_1978(y, m, d)
    int y, m, d;
{
    unsigned n;
    int i;

    if (y < 1978 || m < 1 || m > 12 || d < 1)
        return 0;
    n = 0;
    for (i = 1978; i < y; i++)
        n += isleap(i) ? 366 : 365;
    for (i = 0; i < m - 1; i++)
        n += month_days[i] + (i == 1 && isleap(y));
    n += d - 1;
    return n;           /* day 0 = 1978-01-01, add 1 for BDOS stamp */
}

/*
 * stamp_to_sysdt: decode a 4-byte BDOS time stamp (DW days since 1978-01-01
 * day 1, DB BCD hour, DB BCD min) into a struct sysdt.  Returns 0 ok, -1 bad.
 */
static int stamp_to_sysdt(stamp, dt)
    char *stamp;
    struct sysdt *dt;
{
    unsigned n;
    int y, m, len;

    n = (stamp[0] & 0xff) | ((stamp[1] & 0xff) << 8);
    if (n == 0)
        return -1;
    n--;                /* days since 1978-01-01 */
    dt->hour = bcd(stamp[2]);
    dt->min  = bcd(stamp[3]);
    dt->sec  = 0;

    for (y = 1978; ; y++) {
        len = isleap(y) ? 366 : 365;
        if (n < len)
            break;
        n -= len;
    }
    for (m = 0; m < 12; m++) {
        len = month_days[m] + (m == 1 && isleap(y));
        if (n < len)
            break;
        n -= len;
    }
    dt->year = y;
    dt->mon  = m + 1;
    dt->day  = n + 1;
    return 0;
}

/*
 * sysdt_to_stamp: encode a struct sysdt into a 4-byte BDOS time stamp.
 */
static void sysdt_to_stamp(dt, stamp)
    struct sysdt *dt;
    char *stamp;
{
    unsigned n;

    n = days_since_1978(dt->year, dt->mon, dt->day) + 1; /* day 1 = 1978-01-01 */
    stamp[0] = n & 0xff;
    stamp[1] = (n >> 8) & 0xff;
    stamp[2] = tobcd(dt->hour);
    stamp[3] = tobcd(dt->min);
}

/*
 * getdatetime: read system clock into sysdt.
 * CP/M-86 1.1 (version 22h): reads from sysdata area.
 * MP/M II and later (version > 22h): uses BDOS fn 105 (T_GET).
 * Returns 0 on success, -1 on failure.
 */
int getdatetime(dt)
    struct sysdt *dt;
{
    char stamp[5];
    int v, s;

    v = bdos(12, 0) & 0xff;
    if (v > 0x22) {
        stamp[0] = stamp[1] = stamp[2] = stamp[3] = stamp[4] = 0;
        s = gettime(stamp);
        if (s < 0)
            return -1;
        dt->sec = bcd(s);
        return stamp_to_sysdt(stamp, dt);
    }
    if (v != 0x22)
        return -1;
    getsysvartime();
    dt->mon  = (tbuf[0]  - '0') * 10 + (tbuf[1]  - '0');
    dt->day  = (tbuf[3]  - '0') * 10 + (tbuf[4]  - '0');
    dt->year = 2000 + (tbuf[6] - '0') * 10 + (tbuf[7] - '0');
    dt->hour = (tbuf[9]  - '0') * 10 + (tbuf[10] - '0');
    dt->min  = (tbuf[12] - '0') * 10 + (tbuf[13] - '0');
    dt->sec  = (tbuf[15] - '0') * 10 + (tbuf[16] - '0');
    return 0;
}

/*
 * setdatetime: set the system clock from sysdt.
 * CP/M-86 1.1 (version 22h): writes directly to the sysdata area.
 * MP/M II and later (version > 22h): uses BDOS fn 104 (T_SET).
 * Returns 0 on success, -1 if unsupported.
 */
int setdatetime(dt)
    struct sysdt *dt;
{
    char stamp[5];
    int v;
    char *p;

    v = bdos(12, 0) & 0xff;
    if (v == 0x22) {
        /* Build "MM/DD/YY\tHH:MM:SS" in tbuf and write to sysdata */
        p = tbuf;
        p[0] = '0' + (dt->mon  / 10);  p[1] = '0' + (dt->mon  % 10);
        p[2] = '/';
        p[3] = '0' + (dt->day  / 10);  p[4] = '0' + (dt->day  % 10);
        p[5] = '/';
        p[6] = '0' + ((dt->year % 100) / 10);
        p[7] = '0' + (dt->year % 10);
        p[8] = '\t';
        p[9]  = '0' + (dt->hour / 10); p[10] = '0' + (dt->hour % 10);
        p[11] = ':';
        p[12] = '0' + (dt->min  / 10); p[13] = '0' + (dt->min  % 10);
        p[14] = ':';
        p[15] = '0' + (dt->sec  / 10); p[16] = '0' + (dt->sec  % 10);
        setsysvartime();
        return 0;
    }
    if (v > 0x22) {
        stamp[4] = tobcd(dt->sec);
        sysdt_to_stamp(dt, stamp);
        return settime(stamp);
    }
    return -1;
}

/*
 * getfdatetime: read file timestamps from an open FCB via BDOS fn 102.
 * which=0: create/access stamp (FCB+18h).
 * which=1: update stamp (FCB+1Ch).
 * Returns 0 on success, -1 on failure or unsupported.
 */
int getfdatetime(fcb, dt, which)
    char *fcb;
    struct sysdt *dt;
    int which;
{
    char *stamp;

    if (getftime(fcb) == 0xff)
        return -1;
    stamp = which ? (fcb + 0x1c) : (fcb + 0x18);
    return stamp_to_sysdt(stamp, dt);
}

/*
 * setfdatetime: set file timestamps in an open FCB via BDOS fn 116 (CP/M-86 v4).
 * create_dt and update_dt may be NULL to leave that stamp unchanged (reads
 * back the current stamp from the FCB first).
 * Returns 0..3 on success, -1 on failure or unsupported.
 */
int setfdatetime(fcb, create_dt, update_dt)
    char *fcb;
    struct sysdt *create_dt;
    struct sysdt *update_dt;
{
    char stamps[8];

    /* preserve existing stamps for any NULL argument */
    if (getftime(fcb) == 0xff)
        return -1;
    if (create_dt)
        sysdt_to_stamp(create_dt, stamps);
    else {
        stamps[0] = fcb[0x18]; stamps[1] = fcb[0x19];
        stamps[2] = fcb[0x1a]; stamps[3] = fcb[0x1b];
    }
    if (update_dt)
        sysdt_to_stamp(update_dt, stamps + 4);
    else {
        stamps[4] = fcb[0x1c]; stamps[5] = fcb[0x1d];
        stamps[6] = fcb[0x1e]; stamps[7] = fcb[0x1f];
    }
    return setftime(fcb, stamps);
}

/*
 * epoch: convert a struct sysdt to Unix epoch (seconds since 1970-01-01 UTC).
 * Returns EPOCH_ERR (-1L) if dt is NULL or out of range (year < 1970 or > 2099).
 */
long epoch(dt)
    struct sysdt *dt;
{
    long days;
    int i;

    if (!dt || dt->year < 1970 || dt->year > 2099
        || dt->mon < 1 || dt->mon > 12 || dt->day < 1
        || dt->day > month_days[dt->mon - 1] + (dt->mon == 2 && isleap(dt->year))
        || dt->hour < 0 || dt->hour > 23
        || dt->min  < 0 || dt->min  > 59
        || dt->sec  < 0 || dt->sec  > 59)
        return EPOCH_ERR;

    days = 0;
    for (i = 1970; i < dt->year; i++)
        days += isleap(i) ? 366 : 365;
    for (i = 0; i < dt->mon - 1; i++)
        days += month_days[i] + (i == 1 && isleap(dt->year));
    days += dt->day - 1;

    return days * 86400L + dt->hour * 3600L + dt->min * 60L + dt->sec;
}

/*
 * epoch_now: read the system clock and return Unix epoch.
 * Returns EPOCH_ERR (-1L) on error.
 */
long epoch_now()
{
    struct sysdt dt;

    if (getdatetime(&dt) < 0)
        return EPOCH_ERR;
    return epoch(&dt);
}
