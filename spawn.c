/* Concurrent program launch. PCTMP supplies the four three-byte ORDER
 * entries. Without that translator the supervisor owns all resolution. */
#include <stdio.h>
#include <string.h>
#include "util.h"

extern int pcorder(char *);
extern int pcclean(void);

/* Return the user owning the open file, or -1. Only system files can
 * be found implicitly on the system drive. Explicit drives stay local. */
extern int pcsysdisk(void);
static int find_file(char *fcb, int user)
{
    int drive;
    if (bdos(15,fcb)!=255) return user;
    if (fcb[0]) return -1;
    drive=pcsysdisk();
    if (drive<1 || drive>16) return -1;
    fcb[0]=drive;
    if (bdos(15,fcb)!=255) {
        if (fcb[10]&128) return user;
        bdos(16,fcb);
    }
    if (user) {
        bdos(32,0);
        if (bdos(15,fcb)!=255) {
            if (fcb[10]&128) return 0;
            bdos(16,fcb);
        }
        bdos(32,user);
    }
    return -1;
}

int p_spawn(char *command)
{
    char order[13], name[16], fcb[36], batch[128];
    char *p, *type;
    char ext[4];
    int user, owner, n, i, explicit_type, rc, oldprio, priority, dos;

    if (!command || strlen(command) > 127) return -1;
    p = command;
    while (*p == ' ' || *p == '\t') ++p;
    for (n = 0; p[n] && p[n] != ' ' && p[n] != '\t'; ++n) {
        if (n >= sizeof(name)-1) return -1;
        name[n] = p[n];
    }
    if (!n) return -1;
    name[n] = 0;
    oldprio = getpriority();
    priority = getppriority()-1;
    if (priority < 64) priority = 64;
    if (priority > 197) priority = 197;
    order[0] = 1;
    user=bdos(32,255);
    owner=user;
    dos = 0;
    if (pcorder(order) == 0) {
        fcbinit(name,fcb);
        explicit_type = fcb[9] != ' ';
        if (explicit_type && strncmp(fcb+9,"CMD",3) && strncmp(fcb+9,"BAT",3) &&
            strncmp(fcb+9,"COM",3) && strncmp(fcb+9,"EXE",3)) return -1;
        for (i = 0; i < 4; ++i) {
            fcbinit(name,fcb);
            type = explicit_type ? fcb+9 : order+1+i*3;
            if (!explicit_type) memcpy(fcb+9,type,3);
            memcpy(ext,type,3);
            type=ext;
            owner=find_file(fcb,user);
            if (owner>=0) {
                dos = !strncmp(type,"COM",3) || !strncmp(type,"EXE",3);
                if (!dos) {
                    bdos(16,fcb);
                    if (!strncmp(type,"BAT",3)) {
                        if (strlen(command)>121) { bdos(32,user); return -1; }
                        strcpy(batch,"BATCH ");
                        strcat(batch,command);
                        command=batch;
                    }
                }
                break;
            }
            if (explicit_type) {
                if (!strncmp(type,"COM",3) || !strncmp(type,"EXE",3))
                    return -1;
                break;
            }
        }
    }
    p_priority(priority);
    rc = dos ? ms_p_cli(fcb,command) : p_cli(command);
    if (!rc) {
        conattach();
        if (dos) pcclean();
    }
    /* The translator consumes the open FCB, as in BATCH. */
    if (dos || owner==0) bdos(32,user);
    p_priority(oldprio);
    return rc;
}
