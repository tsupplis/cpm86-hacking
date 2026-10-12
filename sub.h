#ifndef SUB_H
#define SUB_H

#define SUB_CREATE  0   /* delete any existing $$$SUB, start fresh          */
#define SUB_APPEND  1   /* push on top of an existing $$$SUB, else create   */

int sub_open(int flags);
int sub_append(char *cmd);
int sub_close(void);
int sub_records(void); /* records in the existing $$$SUB (0 if none); writes nothing */
int sub_abort(void);   /* undo since sub_open: delete if created, else restore rc */
int sub_delete(void);  /* BDOS 19: delete $$$SUB if it exists */
void sub_exit(void);       /* BDOS fn 0: warm boot, CCP picks up $$$SUB automatically */
void p_chain(char *cmd, int submode); /* BDOS 47: chain; submode!=0 sets MDSUBE so $$$SUB runs after */
int  sub_cmddrv(void);     /* drive (1=A..16=P) prefixing the running CCP command, 0 = none/unknown */
int  sub_active(void);     /* CCP submit mode: 1 = on, 0 = off, -1 = unknown CCP */

/* .sub reader (subfile.c) -- DR SUBMIT rules: $1..$9, $$, ^A..^Z,
   blanks trimmed, blank lines and lines whose first non-blank is ;
   skipped (a ; later in a line is kept), stops at ^Z */
#define SUB_MAX_LINES  64    /* lines per .sub file                       */
#define SUB_LINE_LEN   126   /* expanded line: CCP limit 125 chars + NUL  */
#define SUB_PARAM_LEN  32    /* chars used per parameter                  */

#define SUBERR_EMPTY    0    /* no commands in the file */
#define SUBERR_OPEN    -1    /* file not found          */
#define SUBERR_LONG    -2    /* expanded line too long  */
#define SUBERR_LINES   -3    /* too many lines          */
#define SUBERR_CTRL    -4    /* bad ^x                  */

int   sub_load(char *cmd);   /* "[d:]file[.typ] [p1 ...]": lines (>0) or SUBERR_* */
char *sub_line(int i);       /* expanded line i, 0 = first line of the file       */
int   sub_lineno(int i);     /* source line number of loaded line i               */
char *sub_name(void);        /* file actually opened, e.g. "B:BACKUP.SUB"         */
int   sub_errline(void);     /* source line of the last error, 0 = none           */
char *sub_errmsg(int err);   /* message for a SUBERR_* code (name to follow)      */
int   sub_pushall(void);     /* push loaded lines on open $$$SUB, last first      */

/* text line reader shared with menudat.c: CR LF, LF or CR line ends, ^Z */
#define SUB_RD_EOF   -1      /* no more text                               */
#define SUB_RD_LONG  -2      /* line longer than the buffer (rest skipped) */
int   sub_readln();          /* (FILE *fp, char *buf, int size): length or SUB_RD_* */

#endif
