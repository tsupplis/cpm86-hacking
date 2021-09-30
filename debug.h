#ifndef _DEBUG_H
#define _DEBUG_H

/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#ifndef __STDC__
unsigned long debug_dump_hex();
#else
unsigned long debug_dump_hex(FILE *fd, char*data, unsigned int size,
    unsigned long offset, int raw);
#endif

#endif
