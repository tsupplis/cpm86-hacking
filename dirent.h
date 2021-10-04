#ifndef _DIRENT_H_
#define _DIRENT_H_

/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

typedef struct _dirent_t {
    unsigned char entry[12+1];
    int drive;
    unsigned long blocks;
    unsigned long drive_blocks;
    unsigned long block_size;
    int is_fat;
    unsigned long fcbs;
    struct _dirent_t * next;
} dirent_t;

#ifndef __STDC__
int dirent_next();
int dirent_first();
int dirent_clear(); 
int dirent_load();
int dirent_is_fat();
#else
int dirent_next(char * fcb, dirent_t * root, dirent_t ** last, int all_extents);
int dirent_first(char * fcb, dirent_t ** root);
int dirent_clear(dirent_t * root);
int dirent_load(char * path, dirent_t ** root,int *ouser, int *odrive, int sort_order,
        int all_extents);
int dirent_is_fat(int drive);

#endif

#define dirent_is_sys(a) (a->entry[10]&~0x7F)
#define dirent_is_ro(a) (a->entry[9]&~0x7F)

#endif
