#include <stdio.h>
#include <string.h>

/* Concurrent queue ABI. Near pointers are offsets in the BDOS work segment. */
#define QMAKE 134
#define QOPEN 135
#define QDELETE 136
#define QTRYREAD 138
#define QTRYWRITE 140
#define QEMPTY 14
#define QFULL 15
#define QNOTFOUND 9
#define MSGLEN 8

static struct qdesc {
    unsigned link;
    unsigned char net, origin;
    unsigned flags;
    char name[8];
    unsigned length, slots, readers, writers, count, head, buffer;
} descriptor;

static struct qparams {
    unsigned char flags, net;
    unsigned address, count;
    char *buffer;
    char name[8];
} params;

static char payload[MSGLEN];
static char message[MSGLEN] = "HELLO";
static int detail;

extern unsigned qcall();

static int queue(fn, block, expected)
int fn, expected;
char *block;
{
    unsigned result;

    result = qcall(fn, block, &detail);
    if ((expected == 0 && result == 0) ||
        (expected != 0 && result == 0xffff && detail == expected))
        return 1;
    printf("BDOS %d: result %04X, error %d (expected %d)\n",
           fn, result, detail, expected);
    return 0;
}

int main()
{
    int ok;
    unsigned version;

    version = qcall(12, 0, &detail);
    if (version != 0x1431 && version != 0x1441) {
        puts("QDEMO requires CCP/M-86 3.1 or Concurrent DOS 4.1.");
        return 1;
    }
    if (sizeof(descriptor) != 28 || sizeof(params) != 16) {
        puts("Unexpected queue structure layout.");
        return 1;
    }
    memcpy(descriptor.name, "QDEMO41 ", 8);
    memcpy(params.name, descriptor.name, 8);
    descriptor.length = MSGLEN;
    descriptor.slots = 1;
    params.count = 1;
    params.buffer = payload;
    if (!queue(QMAKE, (char *)&descriptor, 0))
        return 1;
    if (!queue(QOPEN, (char *)&params, 0)) {
        puts("Cannot obtain a handle to delete QDEMO41.");
        return 1;
    }

    memcpy(payload, message, MSGLEN);
    ok = queue(QTRYREAD, (char *)&params, QEMPTY) &&
         queue(QTRYWRITE, (char *)&params, 0) &&
         queue(QTRYWRITE, (char *)&params, QFULL);
    if (ok) {
        memset(payload, 0, MSGLEN);
        ok = queue(QTRYREAD, (char *)&params, 0);
        if (ok && memcmp(payload, message, MSGLEN) != 0) {
            puts("Queue payload mismatch.");
            ok = 0;
        }
        if (ok)
            ok = queue(QTRYREAD, (char *)&params, QEMPTY);
    }
    if (!queue(QDELETE, (char *)&params, 0))
        return 1;
    if (!queue(QOPEN, (char *)&params, QNOTFOUND) || !ok)
        return 1;
    puts("QDEMO PASS: empty, full, payload, deleted");
    return 0;
}
