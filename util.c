#include <stdio.h>
#include <ctype.h>
#include <util.h>

#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#endif

/*#define DEBUG_LOW */

#define VALLEN 256
#define VARLEN 16

typedef struct _entry_t {
    char * var;
    char * val;
    struct _entry_t * next;
} entry_t;

static entry_t * le_entries=0;
static int loaded=0;

#define STATE_INIT      0
#define STATE_NL        1
#define STATE_VAR       2
#define STATE_VAL       3
#define STATE_PRESEP    4
#define STATE_POSTSEP   5
#define STATE_ERROR     6
#define STATE_EOF       7       
#define STATE_COMMENT   8       

#ifndef __STDC__
static st_val(c,val,state,offset) 
    int c;
    char * val;
    int * state;
    int * offset;
#else
static void st_val(int c, char * val, int * state, int * offset) 
#endif
{
#ifdef DEBUG_LOW
    fprintf(stderr,"DBG: %d:%d[%c]\n",*state,*offset,(char)c>=' '?c:'.');
#endif
    if(c==EOF) {
        val[*offset]=0;
        *offset=0;
        *state=STATE_EOF;
        return;
    }
    if(c=='\n' || c=='\r') {
        val[*offset]=0;
        *offset=0;
        *state=STATE_NL;
        return;
    }
    if(*offset<VALLEN) {
        val[*offset]=c;
        *offset=*offset+1;
    }
    return;
}

#ifndef __STDC__
static st_postsep(c,val,state,offset) 
    int c;
    char * val;
    int * state;
    int * offset;
#else
static void st_postsep(int c, char * val, int * state, int * offset) 
#endif
{
#ifdef DEBUG_LOW
    fprintf(stderr,"DBG: %d:%d[%c]\n",*state,*offset,(char)c>=' '?c:'.');
#endif
    if(c==' ' || c=='\t') {
        return;
    }
    if(c==EOF) {
        val[*offset]=0;
        *offset=0;
        *state=STATE_EOF;
        return;
    }
    if(c=='\n' || c=='\r') {
        val[*offset]=0;
        *offset=0;
        *state=STATE_NL;
        return;
    }
    val[*offset]=c;
    *offset=*offset+1;
    *state=STATE_VAL;
    return;
}

#ifndef __STDC__
static st_presep(c,state,offset) 
    int c;
    int * state;
    int * offset;
#else
static void st_presep(int c, int * state, int * offset) 
#endif
{
#ifdef DEBUG_LOW
    fprintf(stderr,"DBG: %d:%d[%c]\n",*state,*offset,(char)c>=' '?c:'.');
#endif
    if(c==' ' || c=='\t') {
        return;
    }
    if(c=='=') {
        *state=STATE_POSTSEP;
        return;
    }
    *state=STATE_ERROR;
    return;
}

#ifndef __STDC__
static st_var(c,var,state,offset) 
    int c;
    char * var;
    int * state;
    int * offset;
#else
static void st_var(int c, char * var, int * state, int * offset) 
#endif
{
#ifdef DEBUG_LOW
    fprintf(stderr,"DBG: %d:%d[%c]\n",*state,*offset,(char)c>=' '?c:'.');
#endif
    if(c==' ' || c=='\t') {
        var[*offset]=0;
        *offset=0;
        if(strlen(var)) {
            *state=STATE_PRESEP;
        } else {
            *state=STATE_ERROR;
        }
        return;
    }
    if(c=='=') {
        var[*offset]=0;
        *offset=0;
        if(strlen(var)) {
            *state=STATE_POSTSEP;
        } else {
            *state=STATE_ERROR;
        }
        return;
    }
    if(isalnum(c)) {
        if(*offset<VARLEN) {
            var[*offset]=toupper(c);
            *offset=*offset+1;
        }
        return;
    }
    if(c=='_') {
        if(*offset<VARLEN) {
            var[*offset]=c;
            *offset=*offset+1;
        }
        return;
    }
    *state=STATE_ERROR;
    return;
}

#ifndef __STDC__
static st_init(c,var,state,offset) 
    int c;
    char * var;
    int * state;
    int * offset;
#else
static void st_init(int c, char * var, int * state, int * offset) 
#endif
{
#ifdef DEBUG_LOW
    fprintf(stderr,"DBG: %d:%d[%c]\n",*state,*offset,(char)c>=' '?c:'.');
#endif
    if(c==';') {
        *state=STATE_COMMENT;
        *offset=0;
        return;
    }
    if(c==' ' || c=='\t') {
        return;
    }
    if(c=='\n' || c=='\t') {
        return;
    }
    if(isalnum(c)) {
        var[*offset]=toupper(c);
        *offset=*offset+1;
        *state=STATE_VAR;
        return;
    }
    if(c=='_') {
        var[*offset]=c;
        *offset=*offset+1;
        *state=STATE_VAR;
        return;
    }
    *state=STATE_ERROR;
    return;
}

#ifndef __STDC__
static st_comment(c,state,offset) 
    int c;
    int * state;
    int * offset;
#else
static void st_comment(int c, int * state, int * offset) 
#endif
{
#ifdef DEBUG_LOW
    fprintf(stderr,"DBG: %d:%d[%c]\n",*state,*offset,(char)c>=' '?c:'.');
#endif
    if(c==EOF) {
        *state=EOF;
    } else if(c=='\n' || c=='\r') {
        *state=STATE_INIT;
    }
}


#ifndef __STDC__
entry_t * new_entry(current,var,val)
    entry_t * current;
    char * var;
    char * val;
#else
entry_t * new_entry(entry_t * current, char * var, char * val)
#endif
{
    int i;
    entry_t * e=(entry_t*)malloc(sizeof(entry_t));
    if(!e) {
        return e;
    }
    e->var=(char *)malloc(strlen(var)+1);
    if(!e->var) {
        free(e);
        return 0;
    }
    strcpy(e->var,var);
    e->val=(char *)malloc(strlen(val)+1);
    if(!e->val) {
        free(e->var);
        free(e);
        return 0;
    }
    strcpy(e->val,val);
    for(i=strlen(e->val)-1;i>-1;i--) {
        if(isspace(e->val[i])) {
            e->val[i]=0;
        }else{
            break;
        }
    }
#ifdef DEBUG
    fprintf(stderr,"DBG: Added entry(%s,%s)\n",e->var,e->val);
#endif
    e->next=current;
    return e;
}

static char lef_var[VARLEN+1];
static char lef_val[VALLEN+1];

#ifndef __STDC__
static int lef(path) 
    char * path;
#else
static int lef(char * path) 
#endif
{
    FILE *fp;
    int state=STATE_INIT;
    int offset=0;
    int loop=1;
    int c;
    entry_t * current;
    char *var=lef_var;    
    char *val=lef_val;    
    current=le_entries;

    fp=fopen(path, "r");
    if(!fp) {
#ifdef DEBUG
        fprintf(stderr,"DBG: Could not find file %s\n",path);
#endif
        return -1;
    }
    while(loop) {
        c=fgetc(fp);
        switch(state) {
        case STATE_INIT:
            st_init(c,var,&state,&offset);
            break;
        case STATE_NL:
            current=new_entry(current,var,val);
            if(current) {
                le_entries=current;
            } else {
                loop=0;
                break;
            }
            state=STATE_INIT;
            st_init(c,var,&state,&offset);
            break;
        case STATE_PRESEP:
            st_presep(c,&state,&offset);
            break;
        case STATE_POSTSEP:
            st_postsep(c,val,&state,&offset);
            break;
        case STATE_COMMENT:
            st_comment(c,&state,&offset);
            break;
        case STATE_VAR:
            st_var(c,var,&state,&offset);
            break;
        case STATE_VAL:
            st_val(c,val,&state,&offset);
            break;
        case STATE_EOF:
            current=new_entry(current,var,val);
            if(current) {
                le_entries=current;
            }
            loop=0;
            break;
        case STATE_ERROR:
            loop=0;
            break;
        }
    }
    fclose(fp);
    return 0;
}

#ifndef __STDC__
int istrcmp (p1, p2, p)
    char *p1;
    char *p2;
    int p;
#else
int istrcmp (char *p1, char *p2,int p)
#endif
{
  unsigned char *s1 = (unsigned char *) p1;
  unsigned char *s2 = (unsigned char *) p2;
  unsigned char c1, c2;

  do
    {
      c1 = (unsigned char) tolower(*s1++);
      c2 = (unsigned char) tolower(*s2++);
      if (c1 == '\0')
        if(p) {
            return 0;
        } else {
            return c1 - c2;
        }
    }
  while (c1 == c2);

  return c1 - c2;
}


#ifndef __STDC__
static le()
#else
static void le()
#endif
{
    if(!loaded) {
        char path[20];
    
        strcpy(path,"0/?:ENV.DAT");

        path[2]=getccpdrv()+'A';
        loaded=!lef(path);
        path[2]=getcurdrv()+'A';
        loaded=!lef(path);
        loaded=1;
    }
}

#ifndef __STDC__
printenv(fp)
    FILE *fp;
#else
void printenv(FILE * fp)
#endif
{
    entry_t * cursor;

    le();
    cursor=le_entries;
    while(cursor) {
        fprintf(fp,"%s=%s\n",cursor->var,cursor->val);
        cursor=cursor->next;
    }
}

#ifndef __STDC__
char* getenv(v)
    char *v;
#else
char* getenv(const char *v)
#endif
{
    entry_t * cursor;

    le();
    cursor=le_entries;
    while(cursor) {
        if(!istrcmp(cursor->var,(char*)v,0)) {
            return cursor->val;
        }
        cursor=cursor->next;
    }
    return 0;
}

