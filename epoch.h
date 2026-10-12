#ifndef EPOCH_H
#define EPOCH_H

/*
 * epoch -- CP/M-86 1.1 system clock -> Unix epoch (epoch.lib).
 *
 * The system date/time lives in the sysdata area as "MM/DD/YY<sep>HH:MM:SS"
 * (BDOS 31h, offset 20h), the same place tod.a86 reads and sets.  YY 00-99
 * means 2000-2099.  Times are treated as UTC.
 */

struct sysdt {
    int year;   /* 2000..2099 */
    int mon;    /* 1..12      */
    int day;    /* 1..31      */
    int hour;   /* 0..23      */
    int min;    /* 0..59      */
    int sec;    /* 0..59      */
};

#define EPOCH_ERR  (-1L)    /* returned by epoch_now()/epoch_make() on error */

int  sys_datetime(struct sysdt *dt);   /* read the clock: 0 ok, -1 not CP/M-86 1.1 or bad date */
long epoch_make(int y, int m, int d, int hh, int mm, int ss);  /* seconds since 1970-01-01, EPOCH_ERR if out of range */
long epoch_now(void);                  /* epoch_make() of the system clock, EPOCH_ERR on error */

#endif
