#ifndef _OS_H_
#define _OS_H_

/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#ifdef __LEGACY__
int keyb();
int bdosx();
#else
int keyb();
int bdosx(int cx, int ax, int* es, int *bx);
#endif

#ifdef __LEGACY__
dpb_load();
#else
dpb_load(int drive, dpb_t*dpb);
#endif


#define dirent_is_sys(a) (a->entry[10]&~0x7F)
#define dirent_is_ro(a) (a->entry[9]&~0x7F)

#endif
