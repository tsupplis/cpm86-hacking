#ifndef _DIRENT_H_
#define _DIRENT_H_

/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

typedef struct _dirent_t {
    unsigned char entry[12+1];
    int drive;
    unsigned long blocks;
    unsigned long fcbs;
    struct _dirent_t * next;
} dirent_t;

typedef struct dpb_t {
    unsigned int spt;
    unsigned char bsh;
    unsigned char blm;
    unsigned char exm;
    unsigned int dsm;
    unsigned int drm;
    unsigned char al0;
    unsigned char al1;
    unsigned int cks;
    unsigned int off;
} dpb_t;


#ifdef __LEGACY__
int dirent_next();
int dirent_first();
int dirent_clear(); 
int dirent_load();
#else
int dirent_next(char * fcb, dirent_t * root, dirent_t ** last, int all_extents);
int dirent_first(char * fcb, dirent_t ** root);
int dirent_clear(dirent_t * root);
int dirent_load(char * path, dirent_t ** root,int *ouser, int *odrive, int sort_order,
        int all_extents);
#endif

#ifdef __LEGACY__
dpb_load();
#else
dpb_load(int drive, dpb_t*dpb);
#endif


#define dirent_is_sys(a) (a->entry[10]&~0x7F)
#define dirent_is_ro(a) (a->entry[9]&~0x7F)

#endif
