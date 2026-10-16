/*
 * epoch -- print the CP/M-86 1.1 system date/time as a Unix epoch.
 * Thin front end over epoch.lib (see epoch.h).
 */
#include <stdio.h>
#include "util.h"

int main()
{
    long t = epoch_now();

    printf("%ld\n", t);
    return 0;
}
