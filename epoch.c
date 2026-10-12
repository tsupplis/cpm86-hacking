/*
 * epochlib -- CP/M-86 1.1 system clock -> Unix epoch.  See epoch.h.
 */
#include "epoch.h"

static char tbuf[18];		/* copy of the sysdata date/time string */

static int month_days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* Copy the 17 bytes at sysdata+20h into tbuf (referenced as tbuf_). */
static void getsys()
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

static int num(p)
    char *p;
{
    return (p[0] - '0') * 10 + (p[1] - '0');
}

static int isleap(y)
    int y;
{
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

int sys_datetime(dt)
    struct sysdt *dt;
{
    if ((bdos(12, 0) & 0xff) != 0x22)		/* CP/M-86 1.1 only */
        return -1;
    getsys();

    dt->mon  = num(tbuf);
    dt->day  = num(tbuf + 3);
    dt->year = 2000 + num(tbuf + 6);
    dt->hour = num(tbuf + 9);
    dt->min  = num(tbuf + 12);
    dt->sec  = num(tbuf + 15);
    return 0;
}

long epoch_make(y, m, d, hh, mm, ss)
    int y, m, d, hh, mm, ss;
{
    long days;
    int i;

    if (y < 1970 || y > 2099 || m < 1 || m > 12 || d < 1
        || d > month_days[m - 1] + (m == 2 && isleap(y))
        || hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59)
        return EPOCH_ERR;

    days = 0;
    for (i = 1970; i < y; i++)
        days += isleap(i) ? 366 : 365;
    for (i = 0; i < m - 1; i++)
        days += month_days[i] + (i == 1 && isleap(y));
    days += d - 1;

    return days * 86400L + hh * 3600L + mm * 60L + ss;
}

long epoch_now()
{
    struct sysdt dt;

    if (sys_datetime(&dt) < 0)
        return EPOCH_ERR;
    return epoch_make(dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);
}
