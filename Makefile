AS=aztec34_as
CC=aztec34_cc
CFLAGS=+F -B +0 -D__CPM86__  -D__LEGACY__
STRIP=aztec34_sqz
LDFLAGS=-lm -lc86
LD=aztec34_link
BINEXT=.cmd
LINKCMD=pcdev_linkcmd
RASM86=pcdev_rasm86

TOOLS=rm.cmd more.cmd write.cmd dump.cmd mode.cmd ls.cmd \
    clsansi.cmd cls.cmd pause.cmd reboot.cmd tod.cmd ver.cmd \
    atinit.cmd attime.cmd

all: binaries

binaries: $(TOOLS)

dist: hack-bin.zip

nix-hack.zip: binaries
	rm -f nix-bin.zip
	zip nix-bin.zip $(TOOLS) 

ls.cmd: ls.o dirent.o bdosx.o debug.o
	$(LD) -o $@ $^ $(LDFLAGS)

rm.cmd: rm.o dirent.o bdosx.o debug.o
	$(LD) -o $@ $^ $(LDFLAGS)

write.cmd: write.o
	$(LD) -o $@ $< $(LDFLAGS)

dump.cmd: dump.o
	$(LD) -o $@ $< $(LDFLAGS)

mode.cmd: mode.o
	$(LD) -o $@ $< $(LDFLAGS)

more.cmd: more.o
	$(LD) -o $@ $< $(LDFLAGS)

bdosx.o: bdosx.asm
	$(AS) $<
	$(STRIP) $@

dir.c: dirent.h debug.h

dirent.c: dirent.h debug.h

debug.c: debug.h

rm.c: dirent.h

ver.cmd: ver.obj
	$(LINKCMD) $<  '[$$sz]'

tod.cmd: tod.obj
	$(LINKCMD) $<  '[$$sz]'

attime.cmd: attime.obj
	$(LINKCMD) $<  '[$$sz]'

atinit.cmd: atinit.obj
	$(LINKCMD) $<  '[$$sz]'

clsansi.cmd: clsansi.obj
	$(LINKCMD) $<  '[$$sz]'

cls.cmd: cls.obj
	$(LINKCMD) $<  '[$$sz]'

pause.cmd: pause.obj
	$(LINKCMD) $< '[$$sz]'

reboot.cmd: reboot.obj
	$(LINKCMD) $< '[$$sz]'

%.obj: %.a86
	$(RASM86) $< $$ pz sz

tod.a86: baselib.a86

ver.a86: baselib.a86

.c.o:
	$(CC) $(CFLAGS) $<
	$(STRIP) $@

clean:
	$(RM) *.o *.h86 *.sym *.prn *.lst *.obj $(TOOLS)

cpmtest.img: $(TOOLS)
	cpmrm -f ibmpc-514ss cpmtest.img 0:*.cmd
	cpmrm -f ibmpc-514ss cpmtest.img 0:test.*
	cpmrm -f ibmpc-514ss cpmtest.img 1:*
	cpmrm -f ibmpc-514ss cpmtest.img 2:*
	cpmcp -f ibmpc-514ss cpmtest.img rm.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img ls.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img write.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img more.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img cls.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img ver.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img tod.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img reboot.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img pause.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img dump.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img mode.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img atinit.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img attime.cmd 0:
	cpmcp -f ibmpc-514ss cpmtest.img test.txt 0:
	cpmcp -f ibmpc-514ss cpmtest.img test.bin 0:
	cpmcp -f ibmpc-514ss cpmtest.img test.txt 1:a
	cpmcp -f ibmpc-514ss cpmtest.img test.txt 1:b
	cpmcp -f ibmpc-514ss cpmtest.img test.txt 2:c
	cpmls -F -f ibmpc-514ss cpmtest.img 0:*.*

test: cpmtest.img
	./cpm86
