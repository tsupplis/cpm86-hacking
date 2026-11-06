#!/usr/bin/env python3
"""Compile source-derived C regressions with Aztec C and execute them in emu2."""
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parent.parent


def command(args, folder):
    return subprocess.run(args, cwd=folder, capture_output=True, timeout=30)


def checked(args, folder):
    result = command(args, folder)
    assert result.returncode == 0, (args, result.stdout, result.stderr)
    return result


def build(folder, sources):
    objects = []
    for source in sources:
        obj = Path(source).with_suffix(".o").name
        checked(["aztec43_acc", "-I.", "-squeeze", "-D__CPM86__",
                 "-c", "-o", obj, source], folder)
        objects.append(obj)
    checked(["aztec43_acc", "-o", "test.cmd"] + objects, folder)


def execute(folder, args=()):
    result = command(["emu2", "test.cmd"] + list(args), folder)
    assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
    assert b"FAIL:" not in result.stdout + result.stderr, (result.stdout, result.stderr)
    output = result.stdout.decode(errors="replace").replace("\r\n", "\n")
    assert "TEST-PASS\n" in output, (result.stdout, result.stderr)
    return output[:output.index("TEST-PASS\n")]


def source(folder, name, replacements=()):
    text = (ROOT / name).read_text()
    for old, new in replacements:
        text = text.replace(old, new)
    (folder / name).write_text(text)


CHECK = r'''
void check(ok, label) int ok; char *label; {
    if (!ok) { fprintf(stderr,"FAIL: %s\n",label); exit(1); }
}
'''


def clock(folder):
    source(folder, "time.c", (("bdos(", "test_bdos("),))
    (folder / "test.c").write_text(r'''
#include <stdio.h>
#include "util.h"
long epoch();
static int stamp_day=1, seconds=0x45, version=0x31;
int test_bdos() { return version; }
int gettime(stamp) char *stamp; {
    stamp[0]=stamp_day; stamp[1]=0; stamp[2]=0x12; stamp[3]=0x34;
    stamp[4]=seconds;
    return seconds;
}
int settime() { return 0; }
int getftime(fcb) char *fcb; {
    fcb[0x18]=1; fcb[0x19]=0; fcb[0x1a]=0x12; fcb[0x1b]=0x34;
    return 0;
}
int setftime() { return 0; }
''' + CHECK + r'''
int main() {
    struct sysdt dt;
    char fcb[36];
    check(getdatetime(&dt)==0 && dt.sec==45 && dt.hour==12 && dt.min==34,
          "Concurrent seconds");
    version=0x41; seconds=0x59;
    check(getdatetime(&dt)==0 && dt.sec==59,"DOS Plus seconds");
    seconds=0;
    check(getdatetime(&dt)==0 && dt.sec==0,"zero seconds");
    stamp_day=0;
    check(getdatetime(&dt)==-1,"invalid system stamp");
    dt.sec=59;
    check(getfdatetime(fcb,&dt,0)==0 && dt.sec==0,"file stamp has no seconds");
    dt.year=1970; dt.mon=1; dt.day=1; dt.hour=dt.min=dt.sec=0;
    check(epoch(&dt)==0L,"epoch origin");
    dt.year=2000; dt.mon=2; dt.day=29;
    check(epoch(&dt)==951782400L,"epoch leap day");
    dt.year=2038; dt.mon=1; dt.day=19; dt.hour=3; dt.min=14; dt.sec=7;
    check(epoch(&dt)==2147483647L,"last signed 32-bit epoch");
    dt.sec=8;
    check(epoch(&dt)==EPOCH_ERR,"first overflowing epoch");
    dt.hour=dt.min=dt.sec=0; dt.day=20;
    check(epoch(&dt)==EPOCH_ERR,"overflowing day");
    dt.year=2040; dt.day=1;
    check(epoch(&dt)==EPOCH_ERR,"2040 rejected, not wrapped");
    dt.year=2099;
    check(epoch(&dt)==EPOCH_ERR,"2099 rejected, not wrapped");
    check(epoch((struct sysdt *)0)==EPOCH_ERR,"null date");
    printf("TEST-PASS\n");
    return 0;
}
''')
    build(folder, ["test.c", "time.c"])
    execute(folder)


def environment(folder):
    source(folder, "util.c", ((name + "(", "test_" + name + "(")
                             for name in ("getusr", "getcurdrv", "getccpdrv")))
    (folder / "test.c").write_text(r'''
#include <stdio.h>
#include "util.h"
int test_getcurdrv() { return 0; }
int test_getccpdrv() { return 0; }
int test_getusr() { return 0; }
int main() { printenv(stdout); printf("TEST-PASS\n"); return 0; }
''')
    build(folder, ["test.c", "util.c"])
    for data, expected in (
        (b"NAME=value\n; trailing comment", "NAME=value\n"),
        (b"; comment only", ""),
        (b";", ""),
        (b"", ""),
        (b"NAME=value", "NAME=value\n"),
        (b"NAME=value\n", "NAME=value\n"),
        (b"; header\nONE=1\nTWO=2\n; trailer", "TWO=2\nONE=1\n"),
    ):
        (folder / "ENV.DAT").write_bytes(data)
        output = execute(folder)
        assert output == expected, (data, output, expected)


def function(text, name, last=False):
    start = text.rindex("int " + name + "(") if last else text.index("int " + name + "(")
    end = text.index("{", start) + 1
    depth = 1
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    return text[start:end]


def terminal_console(folder):
    text = (ROOT / "conio.c").read_text().replace("bdos(", "test_bdos(")
    text, count = re.subn(
        r"#asm(.*?)#endasm",
        lambda match: "test_video_mode(%d);" %
        (3 if "mov al, 3" in match.group(1) else 0),
        text, flags=re.S,
    )
    assert count == 2, "expected two BIOS video-mode blocks"
    (folder / "conio.c").write_text(text)
    (folder / "test.c").write_text(r'''
#include <stdio.h>
#include <string.h>
static unsigned char output[256];
static int used, version, system_type, resets, status, video;
static int status_writes;
static char status_text[14];
int test_bdos(fn, arg) int fn, arg; {
    if (fn == 2) output[used++] = arg;
    return 0;
}
int osver() { return version; }
int ostype() { return system_type; }
int dosplus_crtreset() { resets++; return 0; }
void xstatline(mode) int mode; { status = mode; }
int test_video_mode(mode) int mode; { video = mode; return 0; }
int segread(segs) int *segs; { return 0; }
int bdosx(fn, arg, es, bx) int fn, arg, *es, *bx; {
    *es = *bx = 0;
    return 0;
}
int pokeb(offset, segment, value) int offset, segment, value; {
    status_text[offset - 0x32] = value;
    status_writes++;
    return 0;
}
#include "conio.c"
''' + CHECK + r'''
void expect(bytes, label) char *bytes, *label; {
    int i;
    if (used != strlen(bytes) || (used && memcmp(output, bytes, used))) {
        fprintf(stderr, "%s: got %d bytes, expected %d:",
                label, used, strlen(bytes));
        for (i = 0; i < used; i++) fprintf(stderr, " %02x", output[i]);
        fprintf(stderr, "\n");
    }
    check(used == strlen(bytes) && (!used || !memcmp(output, bytes, used)), label);
    used = 0;
}
int main() {
    int i, j;
    static int versions[] = {0x21, 0x22, 0x30, 0x31, 0x32, 0x41, 0x60};
    static char position[] = {27, 'Y', 35, 39, 0};
    static char foreground[] = {27, 'j', 27, 'b', 15, 27, 'k', 0};
    static char truncated[] = {27, 'j', 27, 'b', 0};

    check(terminal.input.keys == 0 && terminal.input.key_count == 0,
          "raw input has no key decoder");
    for (i = 0; i < 7; i++) {
        version = versions[i];
        for (j = 0; j < 2; j++) {
            system_type = j ? OSTYPE_MULTIUSER : 0;
            resets = 0;
            crtreset();
            expect(version > 0x22 ? "\033e\033z" :
                   "\033m\0330\0331\033q\033t\033u", "reset bytes");
            check(resets == (version == 0x41 && !j), "DOS Plus reset");
        }
        cursor(CURSOR_ON);
        expect(version > 0x22 ? "\x1be" : "\x1bm", "cursor on");
        cursor(CURSOR_OFF);
        expect(version > 0x22 ? "\x1bf" : "\x1bn", "cursor off");
        cursor(CURSOR_SAVE); expect("\x1bj", "cursor save");
        cursor(CURSOR_RESTORE); expect("\x1bk", "cursor restore");
        cursor(99); expect("", "unknown cursor command");
        status = 0;
        statline(STATLINE_ON);
        expect(version == 0x22 ? "\x1b1" : "", "status on bytes");
        check(status == (version > 0x31 ? 2 : 0), "status on service");
        status = 0;
        statline(STATLINE_OFF);
        expect(version == 0x22 ? "\x1b0" : "", "status off bytes");
        check(status == (version > 0x31 ? 1 : 0), "status off service");
    }
    clrscr(); expect("\x1bE", "clear screen");
    clreos(); expect("\x1bJ", "clear screen tail");
    clreol(); expect("\x1bK", "clear line tail");
    insline(); expect("\x1bL", "insert line");
    delline(); expect("\x1bM", "delete line");
    delchar(); expect("\x1bN", "delete character");
    wrapline(1); expect("\x1bv", "wrap on");
    wrapline(0); expect("\x1bw", "wrap off");
    gotoxy(3, 7); expect(position, "unchanged coordinate order");
    gotoxy(-32, 7); expect("\033Y", "zero coordinate byte");
    textcolor(15); expect(foreground, "foreground");
    foreground[4] = (char)255;
    textcolor(255); expect(foreground, "high-bit foreground");
    textcolor(256); expect(truncated, "zero color byte");
    textcolor(0); textcolor(-1); expect("", "ignored foreground");
    version = 0x22;
    system_type = 0;
    video = -1;
    scrmode(SCRMODE_COL80);
    expect("\x1b0\x1b1\x1bE", "80-column bytes");
    check(video == 3, "80-column BIOS mode");
    scrmode(SCRMODE_COL40);
    expect("\x1b0\x1bE", "40-column bytes");
    check(video == 0, "40-column BIOS mode");
    scrmode(SCRMODE_MONO); expect("\x1by", "monochrome");
    scrmode(SCRMODE_COLOR); expect("\x1bx", "color");
    scrmode(SCRMODE_DEFAULT);
    expect("\033m\0330\0331\033q\033t\033u\x1bE", "default mode");
    scrmode(99); expect("", "unknown screen mode");
    version = 0x41;
    video = -1;
    for (i = 0; i <= 8; i++) scrmode(i);
    expect("", "screen modes ignored on later OS");
    check(video == -1, "no later BIOS mode change");
    status_writes = 0;
    setstatus("ignored");
    check(status_writes == 0, "later status text ignored");
    version = 0x22;
    setstatus("Ab\\uCd\\lEf\\sGh");
    check(status_writes == 14 && !memcmp(status_text, "abCdef gh     ", 14),
          "status text unchanged");
    printf("TEST-PASS\n");
    return 0;
}
''')
    build(folder, ["test.c"])
    execute(folder)


def keyboard(folder):
    for name, dos in (("conio.c", False), ("getch.c", False), ("getch.c", True)):
        text = (ROOT / name).read_text()
        declaration = re.search(r"^static char .*getch_buffer.*;", text, re.M).group()
        if dos:
            body = function(text[text.index("int getch()"):], "getch")
        else:
            body = function(text, "getch", last=True)
        body = body.replace("malloc(", "test_malloc(")
        (folder / "test.c").write_text(r'''
#include <stdio.h>
#define GETCH_BUFLEN 64
''' + declaration + CHECK + r'''
char *test_malloc() {
    check(0,"keyboard must not allocate from heap");
    return 0;
}
static int pos;
int next_key(empty) int empty; {
    int step=pos++%5;
    if(step==4) return empty;
    if(step==0) return 0x41;
    if(step==1) return 0xc8;
    if(step==2) return 0xd0;
    return 0xff;
}
int bdos() { return next_key(0); }
int dos_key() { return next_key(0xffff); }
''' + body + r'''
int main() {
    int i;
    for(i=0;i<100;i++) {
        check(getch()==0x41,"direct byte");
        check(getch()==0xc8,"buffered high byte");
        check(getch()==0xd0,"buffered high byte");
        check(getch()==0xff,"buffered FF byte");
    }
    printf("TEST-PASS\n");
    return 0;
}
''')
        # Rename only the source's BDOS calls, not the C runtime's console I/O.
        path = folder / "test.c"
        path.write_text(re.sub(r"\bbdos\(", "test_bdos(", path.read_text()))
        build(folder, ["test.c"])
        execute(folder)


def word_count(folder):
    source(folder, "wc.c", (("main(", "wc_main("), ("malloc(", "test_malloc("),
                            ("fclose(", "test_fclose(")))
    path = folder / "wc.c"
    path.write_text("char *test_malloc();\n" + path.read_text())
    cases = (
        ("LF.TXT", b"one\ntwo\n", 2, 2),
        ("CRLF.TXT", b"one\r\ntwo\r\n", 2, 2),
        ("CR.TXT", b"one\rtwo\r", 2, 2),
        ("MIXED.TXT", b"one\r\ntwo\nthree\rfour", 4, 3),
        ("EMPTY.TXT", b"", 0, 0),
        ("SPLIT.TXT", b"x" * 16383 + b"\r\ny", 2, 1),
    )
    assertions = []
    for name, data, words, lines in cases:
        (folder / name).write_bytes(data + b"\x1a")
        assertions.append(f'''
 check(parse_file("{name}",&w,&l,&c)==0,"read {name}");
 check(w=={words}L && l=={lines}L && c=={len(data)}L,"counts {name}");
''')
    (folder / "test.c").write_text(r'''
#include <stdio.h>
#include "dirent.h"
char *malloc();
static int fail_alloc, closes;
char *test_malloc(size) int size; { return fail_alloc ? 0 : malloc(size); }
int test_fclose(fp) FILE *fp; { closes++; return fclose(fp); }
int dirent_load() { return 255; }
int dirent_free() { return 0; }
''' + CHECK + r'''
int main() {
    unsigned long w,l,c;
''' + "".join(assertions) + r'''
    closes=0; fail_alloc=1;
    check(parse_file("LF.TXT",&w,&l,&c)!=0,"allocation failure reported");
    check(closes==1,"input closed on allocation failure");
    printf("TEST-PASS\n");
    return 0;
}
''')
    build(folder, ["test.c", "wc.c"])
    execute(folder)


def directory(folder):
    source(folder, "wc.c", (("main(", "wc_main("), ("exit(", "test_exit("),
                            ("int parse_file(", "int unused_parse_file("),
                            ("if(parse_file(", "if(test_parse_file(")))
    (folder / "test.c").write_text(r'''
#include <stdio.h>
#include "dirent.h"
static dirent_t entries[3];
static int loads, frees, parses, all;
''' + CHECK + r'''
int dirent_load(path,root) char *path; dirent_t **root; {
    int i,j;
    loads++;
    for(i=0;i<3;i++) {
        for(j=0;j<sizeof(dirent_t);j++) ((char *)&entries[i])[j]=0;
        memcpy(entries[i].entry+1,"FILE    TXT",11);
        entries[i].entry[5]='1'+i;
        entries[i].next=i==2?0:&entries[i+1];
    }
    entries[1].is_hidden=1;
    entries[2].entry[10]|=0x80;
    *root=entries;
    return 0;
}
int dirent_free() { frees++; return 0; }
int test_parse_file(path,w,l,c) char *path; unsigned long *w,*l,*c; {
    parses++; *w=*l=*c=1; return 0;
}
void test_exit(code) int code; {
    check(code==0,"WC main status");
    check(loads==1,"one directory load per pattern");
    check(frees==1,"one directory free per pattern");
    check(parses==(all?3:1),"hidden/system filtering");
    printf("TEST-PASS\n");
    exit(0);
}
int main(argc,argv) int argc; char **argv; {
    char *args[4];
    all=argc>1;
    args[0]="wc"; args[1]=all?"-a":"*.*"; args[2]="*.*"; args[3]=0;
    wc_main(all?3:2,args);
    check(0,"WC main must exit");
    return 1;
}
''')
    build(folder, ["test.c", "wc.c"])
    execute(folder)
    execute(folder, ("all",))


def dump(folder):
    source(folder, "dump.c", (("main(", "dump_main("), ("malloc(", "test_malloc(")))
    path = folder / "dump.c"
    path.write_text("char *test_malloc();\n" + path.read_text())
    source(folder, "debug.c", (("/*#define DEBUG*/", "#define DEBUG"),))
    (folder / "test.c").write_text(r'''
#include <stdio.h>
#include "debug.h"
unsigned long dump_hex();
char *test_malloc() { return 0; }
''' + CHECK + r'''
int main() {
    char value='A';
    char *args[3];
    check(dump_hex(stdout,&value,1,0x12345L,0)==0x12346L,"dump offset");
    check(debug_dump_hex(stdout,&value,1,0x12345L,0)==0x12346L,"debug offset");
    args[0]="dump"; args[1]="INPUT.BIN"; args[2]=0;
    check(dump_main(2,args)!=0,"dump allocation failure reported");
    printf("TEST-PASS\n");
    return 0;
}
''')
    (folder / "INPUT.BIN").write_bytes(b"A")
    build(folder, ["test.c", "dump.c", "debug.c"])
    assert execute(folder).count("012345: ") == 2, "long hex offsets"


def colors(folder):
    source(folder, "mode.c", (("main(", "mode_main("),))
    (folder / "test.c").write_text(r'''
#include <stdio.h>
int setstatus() { return 0; }
int scrmode() { return 0; }
int clrscr() { return 0; }
int cursor() { return 0; }
int statline() { return 0; }
int osver() { return 0x22; }
int istrcmp() { return 0; }
''' + CHECK + r'''
int main() {
    char arg[2], saved;
    int i;
    arg[1]=0;
    for(i=0;i<6;i++) {
        saved=arg[0]='A'+i;
        check(bg(arg)==0 && arg[0]==saved,"uppercase color/input unchanged");
        saved=arg[0]='a'+i;
        check(bg(arg)==0 && arg[0]==saved,"lowercase color/input unchanged");
    }
    printf("TEST-PASS\n");
    return 0;
}
''')
    build(folder, ["test.c", "mode.c"])
    assert execute(folder) == "".join("\x1bc" + chr(i) for i in range(10, 16)
                                      for _ in range(2))


def check():
    tests = (clock, environment, terminal_console, keyboard, word_count,
             directory, dump, colors)
    selected = sys.argv[1:]
    for test in tests:
        if selected and test.__name__ not in selected:
            continue
        with tempfile.TemporaryDirectory(prefix="c-regression-") as work:
            folder = Path(work)
            for name in ("util.h", "conio.h", "dirent.h", "debug.h"):
                shutil.copyfile(ROOT / name, folder / name)
            test(folder)
        print("PASS:", test.__name__)


if __name__ == "__main__":
    check()
