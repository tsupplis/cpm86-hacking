/*
 * subfile.c -- .sub file reader for sub.lib (used by MENU and MSUB).
 * Licensed under the MIT license. See LICENSE file in the project root.
 */
#include <stdio.h>
#include "sub.h"

/*
 * sub_load: read a .sub file and expand it into sub_lines[] using the
 * DR SUBMIT rules: $1..$9 = parameters (missing = empty), $$ = '$',
 * ^A..^Z = control character, leading blanks skipped, trailing blanks
 * trimmed, blank lines dropped. Lines are upcased like SUBMIT/CCP do.
 *
 * cmd format: "[D:]FILENAME[.TYP] [param1 [param2 ...]]"
 * ".SUB" is added when the name has no type.
 *
 * Returns the number of lines (>0) or one of the SUBERR_* codes.
 * The whole file is read and checked before anything is written, so a
 * caller can report errors without touching $$$.SUB.
 */
/*
 * sub_readln: read one text line from fp into buf (size includes the NUL).
 * A line ends with CR LF, LF or CR alone; ^Z or end of file ends the text.
 * Returns the line length, SUB_RD_EOF when there is no more text, or
 * SUB_RD_LONG when the line did not fit (rest of it skipped, buf holds the
 * start).
 */
int sub_readln(fp, buf, size)
    FILE *fp;
    char *buf;
    int   size;
{
    int c;
    int n;
    int over;

    n = 0;
    over = 0;
    c = getc(fp);
    if (c == EOF || c == 0x1A)
        return SUB_RD_EOF;
    while (c != EOF && c != 0x1A && c != '\r' && c != '\n') {
        if (n < size - 1) buf[n++] = (char)c; else over = 1;
        c = getc(fp);
    }
    if (c == '\r') {
        c = getc(fp);               /* CR LF counts as one line end */
        if (c != '\n' && c != EOF)
            ungetc(c, fp);
    } else if (c == 0x1A) {
        ungetc(c, fp);              /* next call reports the end */
    }
    buf[n] = '\0';
    return over ? SUB_RD_LONG : n;
}

static char sub_lines[SUB_MAX_LINES][SUB_LINE_LEN];
static char sub_fname[16];  /* d:filename.typ + NUL */
static int  sub_nlines;     /* lines loaded by the last sub_load       */
static int  sub_srcline;    /* source line of the last error, 0 = none */
static int  sub_lineno_[SUB_MAX_LINES]; /* source line of each loaded line */

int sub_load(cmd)
    char *cmd;
{
    FILE *fp;
    char  params[9][SUB_PARAM_LEN + 1];
    int   nparams;
    int   nlines;
    char  buf[256];
    char *out;
    char *p;
    char *q;
    char *s;
    int   i;
    int   j;
    int   base;
    int   pn;
    int   err;

    /* --- file name: whole token, extra characters ignored --- */
    p = cmd;
    while (*p == ' ' || *p == '\t') p++;
    i = 0;
    while (*p != '\0' && *p != ' ' && *p != '\t') {
        if (i < 14) {
            sub_fname[i] = *p;
            if (*p >= 'a' && *p <= 'z') sub_fname[i] = *p - 'a' + 'A';
            i++;
        }
        p++;
    }
    sub_fname[i] = '\0';
    base = (i >= 2 && sub_fname[1] == ':') ? 2 : 0;
    {
        int hasdot = 0;
        for (j = base; j < i; j++) if (sub_fname[j] == '.') { hasdot = 1; break; }
        if (!hasdot && i - base <= 8) {
            sub_fname[i]='.'; sub_fname[i+1]='S'; sub_fname[i+2]='U';
            sub_fname[i+3]='B'; sub_fname[i+4]='\0';
        }
    }

    /* --- parameters $1..$9: whole tokens, extra characters ignored --- */
    nparams = 0;
    while (*p != '\0' && nparams < 9) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;
        j = 0;
        while (*p != '\0' && *p != ' ' && *p != '\t') {
            if (j < SUB_PARAM_LEN) params[nparams][j++] = *p;
            p++;
        }
        params[nparams][j] = '\0';
        nparams++;
    }

    sub_nlines  = 0;
    sub_srcline = 0;
    fp = fopen(sub_fname, "r");
    if (fp == 0) return SUBERR_OPEN;

    nlines = 0;
    err = 0;
    while ((i = sub_readln(fp, buf, sizeof(buf))) != SUB_RD_EOF) {
        sub_srcline++;
        if (i == SUB_RD_LONG) { err = SUBERR_LONG; break; }

        q = buf;
        while (*q == ' ' || *q == '\t') q++;
        /* blank line, or ; as first non-blank = comment; a later ; is text */
        if (*q == '\0' || *q == ';') continue;

        if (nlines >= SUB_MAX_LINES) { err = SUBERR_LINES; break; }
        out = sub_lines[nlines];

        /* expand into out[] */
        j = 0;
        while (*q != '\0' && err == 0) {
            if (q[0] == '$' && q[1] == '$') {
                if (j < SUB_LINE_LEN - 1) out[j++] = '$'; else err = SUBERR_LONG;
                q += 2;
            } else if (q[0] == '$' && q[1] >= '1' && q[1] <= '9') {
                pn = q[1] - '1';
                if (pn < nparams) {
                    for (s = params[pn]; *s != '\0'; s++) {
                        if (j < SUB_LINE_LEN - 1) out[j++] = *s;
                        else { err = SUBERR_LONG; break; }
                    }
                }
                q += 2;
            } else if (q[0] == '^') {
                char c = q[1];
                if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
                if (c < 'A' || c > 'Z') { err = SUBERR_CTRL; break; }
                if (j < SUB_LINE_LEN - 1) out[j++] = (char)(c - 'A' + 1); else err = SUBERR_LONG;
                q += 2;
            } else {
                if (j < SUB_LINE_LEN - 1) out[j++] = *q; else err = SUBERR_LONG;
                q++;
            }
        }
        if (err != 0) break;

        /* trim trailing blanks, drop lines left empty */
        while (j > 0 && (out[j-1] == ' ' || out[j-1] == '\t')) j--;
        out[j] = '\0';
        if (j == 0) continue;
        sub_lineno_[nlines] = sub_srcline;

        for (i = 0; i < j; i++)
            if (out[i] >= 'a' && out[i] <= 'z') out[i] = out[i] - 'a' + 'A';
        nlines++;
    }
    fclose(fp);

    if (err != 0) return err;
    sub_srcline = 0;
    sub_nlines = nlines;
    return nlines;
}


char *sub_line(i)
    int i;
{
    if (i < 0 || i >= sub_nlines) return "";
    return sub_lines[i];
}

int sub_lineno(i)
    int i;
{
    if (i < 0 || i >= sub_nlines) return 0;
    return sub_lineno_[i];
}

char *sub_name()
{
    return sub_fname;
}

int sub_errline()
{
    return sub_srcline;
}

char *sub_errmsg(n)
    int n;
{
    switch (n) {
    case SUBERR_OPEN:  return "Cannot open ";
    case SUBERR_LONG:  return "Line over 125 chars in ";
    case SUBERR_LINES: return "Too many lines in ";
    case SUBERR_CTRL:  return "Bad ^ control char in ";
    }
    return "No commands in ";
}

/* push the loaded lines on the open $$$.SUB, last line first, so the CCP
   runs them in file order */
int sub_pushall()
{
    int i;

    for (i = sub_nlines - 1; i >= 0; i--)
        if (sub_append(sub_lines[i]) != 0)
            return -1;
    return 0;
}
