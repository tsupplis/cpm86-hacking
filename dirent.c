/*
  Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <stdio.h>
#ifndef __LEGACY__
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

#ifdef __LEGACY__
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
    fprintf(stdout,"->fcb\n",rc);
    debug_dump_hex(stdout, (unsigned char*)(fcb), 33, 0UL, 0);
    fprintf(stdout,"->%d\n",rc);
    debug_dump_hex(stdout, (unsigned char*)(0x80), 32*4, 0UL, 0);
    getchar();
#endif
    cursor=root;
    while(cursor) {
        if(!memcmp(f+1,(cursor->entry)+1,11)) {
            {
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
    {
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

#ifdef __LEGACY__
int dirent_first(fcb, root) 
    char * fcb;
    dirent_t ** root;
#else
int dirent_first(char * fcb, dirent_t ** root)
#endif
{
    int rc=0;
    unsigned char* f=0;

    rc=bdos(17,fcb);
    if(rc==255) {
        *root=0;
        return rc;
    }
    f=(unsigned char*)(0x80+rc*32);
#ifdef DEBUG
    fprintf(stdout,"->fcb\n",rc);
    debug_dump_hex(stdout, (unsigned char*)(fcb), 33, 0UL, 0);
    fprintf(stdout,"->%d\n",rc);
    debug_dump_hex(stdout, (unsigned char*)(0x80), 32*4, 0UL, 0);
    getchar();
#endif
    root[0]=(dirent_t*)malloc(sizeof(dirent_t));
    if(!root[0]) {
        return -2;
    }
    memset(root[0],0,sizeof(dirent_t));
    memcpy(root[0]->entry,f,12);
    root[0]->drive=fcb[0];
    {
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


#ifdef __LEGACY__
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

#ifdef __LEGACY__
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

#ifdef __LEGACY__
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

#ifdef __LEGACY__
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

#ifdef __LEGACY__
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
        fcb[0]=bdos(25,0)+1;
    }
    if(odrive) {
        *odrive=fcb[0];
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
    fprintf(stdout,"->fcb\n",rc);
    debug_dump_hex(stdout, (unsigned char*)(fcb), 33, 0UL, 0);
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
    rstusr();
    if(sort_order) 
        dirent_sort(root,sort_order);
    return rc;
}

#ifdef __LEGACY__
dpb_load(drive, dpb)
    int drive; 
    dpb_t* dpb; 
#else
dpb_load(int drive, dpb_t*dpb) 
#endif
{
    int i;
    int es;
    int bx;
    int ax;
    int segs[4];
    int curdrive=bdos(25,0);
    unsigned char *ptr=(unsigned char *)dpb;

    bdos(14,drive-1);
    segread(segs);
    ax=bdosx(31,0,&es,&bx);
    for(i=0;i<15;i++) 
        ptr[i]=peekb(bx+2,es);
    bdos(14,curdrive);
}


