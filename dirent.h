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
    int is_dir;
    int include_dirs;
    unsigned long fcbs;
    struct _dirent_t * next;
} dirent_t;

#ifndef __STDC__
int dirent_next();
int dirent_first();
int dirent_free();
int dirent_load();
int dirent_is_fat();
int dirent_fcb();
void dirent_sort();
#else
int dirent_next(char * fcb, dirent_t * root, dirent_t ** last, int dirent_options);
int dirent_first(char * fcb, dirent_t ** root);
int dirent_free(dirent_t * root);
int dirent_load(char * path, dirent_t ** root,int *ouser, int *odrive, int sort_order,
        int dirent_options);
int dirent_is_fat(int drive);
int dirent_fcb(char *fcb,dirent_t *d);
void dirent_sort(dirent_t ** root, int sort_order);

#endif

#define DIRENT_ALL_EXTENTS 1
#define DIRENT_INCLUDE_DIRS 2
#define dirent_is_dir(a) ((a)->is_dir)

#define dirent_is_sys(a) (a->entry[10]&~0x7F)
#define dirent_is_ro(a) (a->entry[9]&~0x7F)

#endif
