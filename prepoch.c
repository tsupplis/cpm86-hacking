/*
 * epoch -- print the CP/M-86 1.1 system date/time as a Unix epoch.
 * Thin front end over epoch.lib (see epoch.h).
 */
#include <stdio.h>
#include "epoch.h"

int main()
{
    long t = epoch_now();

    if (t == EPOCH_ERR) {
        fputs("Requires CP/M-86 1.1 and a valid system date\n", stderr);
        return 1;
    }
    printf("%ld\n", t);
    return 0;
}
