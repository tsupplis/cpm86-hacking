AS=aztec34_as
CC=aztec34_cc
CFLAGS=+F -B +0 -D__CPM86__  -D__LEGACY__
STRIP=aztec34_sqz
LDFLAGS=-lm -lc86
LD=aztec34_link
LINK86=pcdev_linkcmd
RASM86=pcdev_rasm86

TOOLS=rm.cmd more.cmd write.cmd dump.cmd mode.cmd ls.cmd \
    clsansi.cmd cls.cmd pause.cmd reboot.cmd tod.cmd ver.cmd \
    atinit.cmd attime.cmd 
PCETOOLS=pce/pceexit.cmd pce/pcever.cmd pce/pcemnt.cmd pce/pcetime.cmd \
    pce/pceinit.cmd

all: binaries

binaries: $(TOOLS) $(PCETOOLS)

dist: hack-bin.zip pce-bin.zip

hack-bin.zip pce-bin.zip: binaries
	rm -f pce-bin.zip
	zip pce-bin.zip $(PCETOOLS) 
	rm -f hack-bin.zip
	zip hack-bin.zip $(TOOLS) 

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

pce/pceexit.cmd pce/pcever.cmd pce/pcemnt.cmd pce/pcetime.cmd \
    pce/pceinit.cmd :
	(cd pce;make all)

clean:
	$(RM) *.o *.h86 *.log *.sym *.prn *.lst *.obj $(TOOLS)
	(cd pce;make clean)


cdostest.img: $(TOOLS) $(PCETOOLS) Makefile
	cp cdosbase.img cdostest.img
	-for i in $(PCETOOLS) $(TOOLS);do \
	    mcopy -o -i cdostest.img $$i ::`basename $$i|tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ` ; \
    done
	mcopy -o -i cdostest.img test.txt ::TEST.TXT
	mcopy -o -i cdostest.img test.bin ::TEST.BIN
	mdir -w -i cdostest.img ::*.*

cpmtest.img: $(TOOLS) $(PCETOOLS) cpmbase.img Makefile test.bin test.txt
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

cpmtest: cpmtest.img
	./cpm86

cdostest: cdostest.img
	./cdos
