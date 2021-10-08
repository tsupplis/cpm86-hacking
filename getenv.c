#include <stdio.h>
#include <os.h>

#ifndef __STDC__
char* getenv(v)
    char *v;
#else
char* getenv(char *v)
#endif
{
    return 0;
}

