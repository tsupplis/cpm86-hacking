#ifndef _DIRENT_H_
#define _DIRENT_H_

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
int dirent_load(char * path, dirent_t ** root,int *ouser, int *odrive. int sort_order,
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
