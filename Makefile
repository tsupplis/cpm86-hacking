AS=aztec34_as
CC=aztec34_cc
AR=aztec34_lib
CFLAGS=-I. +F -B +0 -D__CPM86__ 
STRIP=aztec34_sqz
LDFLAGS=-lc86
LD=aztec_link
LINK86=pcdev_linkcmd
RASM86=pcdev_rasm86

TOOLS=rm.cmd more.cmd write.cmd dump.cmd mode.cmd ls.cmd \
    clsansi.cmd cls.cmd pause.cmd reboot.cmd tod.cmd ver.cmd \
    atinit.cmd attime.cmd ciotest.cmd ball.cmd getch.cmd
PCETOOLS=pce/pceexit.cmd pce/pcever.cmd pce/pcemnt.cmd pce/pcetime.cmd \
    pce/pceinit.cmd

all: binaries

binaries: $(TOOLS) 
	(cd pce;make binaries)

dist: hack-bin.zip pce-bin.zip

hack-bin.zip pce-bin.zip: binaries
	rm -f pce-bin.zip
	zip pce-bin.zip $(PCETOOLS) 
	rm -f hack-bin.zip
	zip hack-bin.zip $(TOOLS) 

getch.cmd: getch.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

ls.cmd: ls.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

ciotest.cmd: ciotest.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

rm.cmd: rm.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

write.cmd: write.o
	$(LD) -o $@ $< $(LDFLAGS)

dump.cmd: dump.o
	$(LD) -o $@ $< $(LDFLAGS)

ball.cmd: ball.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

mode.cmd: mode.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

more.cmd: more.o
	$(LD) -o $@ $< $(LDFLAGS)

util.lib: conio.o dirent.o dpb.o debug.o os.o gfx.o
	rm -f $@
	$(AR) $@ $^

gfx.o: gfx.asm
	$(AS) $<
	$(STRIP) $@

os.o: os.asm
	$(AS) $<
	$(STRIP) $@

ls.c: dirent.h debug.h

dirent.c: dirent.h debug.h

debug.c: debug.h

rm.c: dirent.h

tod.a86: baselib.a86

ver.a86: baselib.a86

attime.a86: baselib.a86 atclock.a86

atinit.a86: baselib.a86 atclock.a86

%.cmd: %.obj
	$(LINK86) $* '[$$sz]'

%.obj: %.a86
	$(RASM86) $< $$ pz sz

%.o: %.c
	$(CC) $(CFLAGS) $<
	$(STRIP) $@

clean:
	$(RM) *.o *.h86 *.log *.sym *.prn *.lst *.obj $(TOOLS) util.lib
	(cd pce;make clean)


dostest.img: binaries Makefile test.bin test.txt
	cp dosbase.img dostest.img
	-for i in $(PCETOOLS) $(TOOLS);do \
	    mcopy -o -i dostest.img $$i ::`basename $$i|tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ` ; \
    done
	mcopy -o -i dostest.img test.txt ::TEST.TXT
	mcopy -o -i dostest.img test.bin ::TEST.BIN
	mdir -w -i dostest.img ::*.*

ccpmtest.img: cpmtest.img startup.0
	cp cpmtest.img ccpmtest.img
	cpmcp -f ibmpc-514ss ccpmtest.img startup.0 0:

cpmtest.img: $(TOOLS) Makefile test.bin test.txt 
	(cd pce;make binaries)
	cp cpmbase.img cpmtest.img
	cpmcp -f ibmpc-514ss cpmtest.img $(PCETOOLS) 0:
	cpmcp -f ibmpc-514ss cpmtest.img $(TOOLS) 0:
	cpmcp -f ibmpc-514ss cpmtest.img test.bin 0:
	cpmcp -f ibmpc-514ss cpmtest.img test.txt 0:
	cpmcp -f ibmpc-514ss cpmtest.img test.txt 1:testa
	cpmcp -f ibmpc-514ss cpmtest.img test.txt 1:testb
	cpmcp -f ibmpc-514ss cpmtest.img test.txt 2:testc
	cpmls -F -f ibmpc-514ss cpmtest.img 0:*.*

test: cpmtest

ccpmtest: ccpmtest.img
	@./ccpm86

cpmtest: cpmtest.img
	@./cpm86

dostest: dostest.img
	@./dosplus
