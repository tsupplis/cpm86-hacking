#ifndef _DEBUG_H
#define _DEBUG_H

/*
     The contents of this file are subject to the Mozilla Public License
     Version 1.0 (the "License"); you may not use this file except in
     compliance with the License. You may obtain a copy of the License at
     http://www.mozilla.org/MPL/

     Software distributed under the License is distributed on an "AS IS"
     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
     License for the specific language governing rights and limitations
     under the License.
*/


#ifdef __LEGACY__
unsigned long debug_dump_hex();
#else
unsigned long debug_dump_hex(FILE *fd, char*data, unsigned int size,
    unsigned long offset, int raw);
#endif

#endif