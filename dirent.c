/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#else
char *malloc();
#endif
#include <os.h>
#include <dirent.h>

/*#define DEBUG*/

#ifdef DEBUG
#include "debug.h"
#endif

#ifndef __STDC__
int dirent_next(fcb, root, last, all_extents) 
    char * fcb;
    dirent_t * root;
    dirent_t ** last;
    int all_extents;
#else
int dirent_next(char * fcb, dirent_t * root, dirent_t ** last, int all_extents)
#endif
{
    int rc=0;
    unsigned char* f=0;
    dirent_t* cursor=0;
    int i;

    rc=bdos(18,fcb);
    if(rc==255) {
        return rc;
    }
    i=rc;
    f=(unsigned char*)(0x80+rc*32);
#ifdef DEBUG
    fprintf(stderr,"dirent_next->fcb\n",rc);
    debug_dump_hex(stderr, (unsigned char*)(fcb), 33, 0L, 0);
    fprintf(stderr,"->%d\n",rc);
    debug_dump_hex(stderr, (unsigned char*)(0x80), 32*4, 0L, 0);
    getchar();
#endif
    cursor=root;
    while(cursor) {
        if(!memcmp(f+1,(cursor->entry)+1,11)) {
            if(root->is_fat) {
                int * s=(int *)(f+28);
                cursor->blocks=(*s+cursor->block_size-1)/cursor->block_size;
            } else if(root->drive_blocks<=256) {
                int j;
                char *blocks;
                blocks=(char*)(f+16);
                for(j=0;j<16;j++) {
                    if(!blocks[j]) 
                        break;
                    cursor->blocks++;
                }
            } else {
                int j;
                int *blocks;
                blocks=(int*)(f+16);
                for(j=0;j<8;j++) {
                    if(!blocks[j]) 
                        break;
                    cursor->blocks++;
                }
            }
            cursor->fcbs++;
            rc=0;
            return rc;
        }
        cursor=cursor->next;
    }
    cursor=(dirent_t*)malloc(sizeof(dirent_t));
    if(!cursor) {
        return -2;
    }
    memset(cursor,0,sizeof(dirent_t));
    memcpy(cursor->entry,f,12);
    cursor->drive=root->drive;
    cursor->block_size=root->block_size;
    cursor->drive_blocks=root->drive_blocks;
    cursor->is_fat=root->is_fat;
    if(root->is_fat) {
        int * s=(int *)(f+28);
        cursor->blocks=(*s+cursor->block_size-1)/cursor->block_size;
    } else if(root->drive_blocks<=256) {
        int j;
        char *blocks;
        blocks=(char*)(f+16);
        for(j=0;j<16;j++) {
            if(!blocks[j]) 
                break;
            cursor->blocks++;
        }
    } else {
        int j;
        int *blocks;
        blocks=(int*)(f+16);
        for(j=0;j<8;j++) {
            if(!blocks[j]) 
                break;
            cursor->blocks++;
        }
    }
    cursor->fcbs=1;
    last[0]->next=cursor;
    last[0]=cursor;
    rc=0;
    return rc;
}

#ifndef __STDC__
int dirent_first(fcb, root) 
    char * fcb;
    dirent_t ** root;
#else
int dirent_first(char * fcb, dirent_t ** root)
#endif
{
    int rc=0;
    unsigned char* f=0;
    int is_fat=dirent_is_fat(fcb[0]-1);

    rc=bdos(17,fcb);
    if(rc==255) {
        *root=0;
        return rc;
    }
    f=(unsigned char*)(0x80+rc*32);
#ifdef DEBUG
    fprintf(stderr,"dirent_first->fcb\n",rc);
    debug_dump_hex(stderr, (unsigned char*)(fcb), 33, 0L, 0);
    fprintf(stderr,"->%d\n",rc);
    debug_dump_hex(stderr, (unsigned char*)(0x80), 32*4, 0L, 0);
    getchar();
#endif
    root[0]=(dirent_t*)malloc(sizeof(dirent_t));
    if(!root[0]) {
        return -2;
    }
    memset(root[0],0,sizeof(dirent_t));
    memcpy(root[0]->entry,f,12);
    root[0]->drive=fcb[0]-1;
    root[0]->is_fat=is_fat;
    {
        dpb_t dpb;
        dpb_load(root[0]->drive,&dpb);
        root[0]->block_size=128<<dpb.bsh;
        root[0]->drive_blocks=dpb.dsm+1;
#ifdef DEBUG
        fprintf(stderr,"disk_info->%lu %lu\n",root[0]->block_size,root[0]->drive_blocks);
        getchar();
#endif
    }
    if(root[0]->is_fat) {
        int * s=(int *)(f+28);
        root[0]->blocks=(*s+root[0]->block_size-1)/root[0]->block_size;
    } else if(root[0]->drive_blocks<=256) 
    {
        int j;
        char *blocks;
        blocks=(char*)(f+16);
        for(j=0;j<16;j++) {
            if(!blocks[j]) 
                break;
            root[0]->blocks++;
        }
    } else {
        int j;
        int *blocks;
        blocks=(int*)(f+16);
        for(j=0;j<8;j++) {
            if(!blocks[j]) 
                break;
            root[0]->blocks++;
        }
    }
    root[0]->fcbs=1;
    rc=0;
    return rc;
}


#ifndef __STDC__
dirent_t* dirent_merge_sorted(lst1,lst2,sort_order) 
    dirent_t* lst1; 
    dirent_t* lst2; 
    int sort_order;
#else
dirent_t* dirent_merge_sorted(dirent_t* lst1, dirent_t* lst2, int sort_order) 
#endif
{ 
	dirent_t* result = 0; 

	if (lst1 == 0) 
		return (lst2); 
	else if (lst2 == 0) 
		return (lst1); 

	if(memcmp(lst1->entry+1,lst2->entry+1,11)*sort_order<0) { 
		result = lst1; 
		result->next = dirent_merge_sorted(lst1->next, lst2, sort_order); 
	} 
	else { 
		result = lst2; 
		result->next = dirent_merge_sorted(lst1, lst2->next, sort_order); 
	} 
	return result; 
} 

#ifndef __STDC__
dirent_split(source, front, back) 
    dirent_t* source; 
    dirent_t** front; 
    dirent_t** back; 
#else
void dirent_split(dirent_t* source, dirent_t** front, dirent_t** back) 
#endif
{ 
	dirent_t* entry1; 
	dirent_t* entry2; 
	entry2 = source; 
	entry1 = source->next; 

	while (entry1 != 0) { 
		entry1 = entry1->next; 
		if (entry1 != 0) { 
			entry2 = entry2->next; 
			entry1 = entry1->next; 
		} 
	} 

	*front = source; 
	*back = entry2->next; 
	entry2->next = 0; 
} 

#ifndef __STDC__
dirent_sort(root, sort_order)
    dirent_t** root; 
    int sort_order;
#else
dirent_sort(dirent_t** root, int sort_order) 
#endif
{ 
	dirent_t* head = *root; 
	dirent_t* entry1; 
	dirent_t* entry2; 

	if ((head == NULL) || (head->next == NULL)) { 
		return; 
	} 

	dirent_split(head, &entry1, &entry2); 

	dirent_sort(&entry1, sort_order); 
	dirent_sort(&entry2, sort_order); 

	*root = dirent_merge_sorted(entry1, entry2, sort_order); 
} 

#ifndef __STDC__
int dirent_clear(root) 
    dirent_t * root;
#else
int dirent_clear(dirent_t * root)
#endif
{
    while(root) {
        dirent_t *temp=root->next;
        free(root);
        root=temp;
    }
    return 0;
}

#ifndef __STDC__
int dirent_is_fat(drive) 
    int drive;
#else
int dirent_is_fat(int drive)
#endif
{
    char fcb[40];
    int rc=0;
    unsigned char* f=0;
    int curdrive=getcurdrv();
        
    setcurdrv(drive);

    fcbinit("",fcb);
    memset(fcb,0,sizeof(fcb));
    memset(fcb,'?',12);
#ifdef DEBUG
    fprintf(stderr,"dirent_is_fat->curdrive %u\n",drive);
    fprintf(stderr,"dirent_is_fat->newdrive %u\n",drive);
    fprintf(stderr,"dirent_is_fat->fcb\n",rc);
    debug_dump_hex(stderr, (unsigned char*)(fcb), 33, 0L, 0);
#endif
    rc=bdos(17,fcb);
    setcurdrv(curdrive);
    if(rc==255) {
        return 0;
    }
    f=(unsigned char*)(0x80+rc*32);
 #ifdef DEBUG
    fprintf(stderr,"->%d\n",rc);
    debug_dump_hex(stderr, (unsigned char*)(0x80), 32*4, 0L, 0);
    getchar();
#endif
    if(f[0x0F]==0x80 && f[0]==' ') {
        return 1;
    }
    return 0;
}

#ifndef __STDC__
int dirent_load(path, root, ouser, odrive, sort_order, all_extents) 
    char * path;
    dirent_t ** root;
    int *ouser;
    int *odrive;
    int sort_order;
    int all_extents;
#else
int dirent_load(char * path, dirent_t ** root,int * ouser,int * odrive, 
    int sort_order,int all_extents)
#endif
{
    char fcb[40];
    int user;
    int rc=0;
    dirent_t * last=0;
    unsigned long count=0;

    user=fcbinit(path,fcb);
    if(user==255) {
        user=getusr();
    }
    if(ouser) {
        *ouser=user;
    }
    if(fcb[0]==0) {
        fcb[0]=getcurdrv()+1;
    }
    if(odrive) {
        *odrive=fcb[0]-1;
    }
    if(fcb[1]==' ') {
        int i;
        for(i=1;i<12;i++) {
            fcb[i]='?';
        }
    }
    if(all_extents) {
        fcb[12]='?';
        fcb[14]='?';
    }
#ifdef DEBUG
    fprintf(stderr,"dirent_load->fcb\n");
    debug_dump_hex(stderr, (unsigned char*)(fcb), 33, 0L, 0);
    getchar();
#endif
    
    setusr(user);
    rc=dirent_first(fcb,root);
    if(rc) {
        return rc;
    }
    last=*root;
    count=1;
    while(!(rc=dirent_next(fcb,*root,&last,all_extents))) {
        count++;
    }
    if(rc==255) {
        rc=0;
    }
#ifdef DEBUG
    fprintf(stderr,"dirent_load->done\n",rc);
#endif
    rstusr();
    if(sort_order) 
        dirent_sort(root,sort_order);
    return rc;
}
