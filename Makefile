AS=aztec34_as
CC=aztec34_cc
AR=aztec34_lib
MASM=pcdev_masm
LINK=pcdev_link
EXE2BIN=pcdev_exe2bin
CFLAGS=-I. -B +0 -D__CPM86__
STRIP=aztec34_sqz
LDFLAGS=-lc86
LD=aztec34_link
LINK86=pcdev_linkcmd
RASM86=pcdev_rasm86

TOOLS=rm.cmd more.cmd write.cmd dump.cmd mode.cmd ls.cmd \
    cls.cmd pause.cmd reboot.cmd tod.cmd ver.cmd touch.cmd wc.cmd \
    atinit.cmd attime.cmd ciotest.cmd ball.cmd getch.cmd \
    printenv.cmd mem.cmd dosver.com
EXTRAS=clsansi.cmd rtctime.cmd rtcinit.cmd
PCETOOLS=pce/pceexit.cmd pce/pcever.cmd pce/pcemnt.cmd pce/pcetime.cmd \
    pce/pceinit.cmd

all: binaries

binaries: $(TOOLS) $(EXTRAS)
	(cd pce;make binaries)

dist: hack-bin.zip pce-bin.zip hack.img

hack-bin.zip pce-bin.zip: binaries
	rm -f pce-bin.zip
	zip pce-bin.zip $(PCETOOLS) 
	rm -f hack-bin.zip
	zip hack-bin.zip $(TOOLS) 

hack.img: cpmtest.img
	cp $< $@

wc.cmd: wc.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

printenv.cmd: printenv.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

getch.cmd: getch.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

ls.cmd: ls.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

ciotest.cmd: ciotest.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

cp.cmd: cp.o util.lib
	$(LD) -o $@ $^ $(LDFLAGS)

touch.cmd: touch.o
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

util.lib: util.o conio.o dirent.o dpb.o debug.o os.o gfx.o
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

tod.obj: tod.a86 baselib.a86 tinylib.a86

ver.obj: ver.a86 tinylib.a86

attime.obj: time.a86 baselib.a86 tinylib.a86 clock.a86
	$(RASM86) $< $$ pz sz iatdef.a86
	mv time.obj attime.obj

rtctime.obj: time.a86 baselib.a86 tinylib.a86 clock.a86
	$(RASM86) $< $$ pz sz irtcdef.a86
	mv time.obj rtctime.obj

mem.obj: mem.a86 tinylib.a86

atinit.obj: init.a86 baselib.a86 tinylib.a86 clock.a86
	$(RASM86) $< $$ pz sz iatdef.a86
	mv init.obj atinit.obj

rtcinit.obj: init.a86 baselib.a86 tinylib.a86 clock.a86
	$(RASM86) $< $$ pz sz irtcdef.a86
	mv init.obj rtcinit.obj

dosver.com: dosver.exe
	$(EXE2BIN) dosver.exe dosver.com

dosver.exe: dosver.obj
	$(LINK) dosver \;

dosver.obj: dosver.asm
	$(MASM) dosver \;

%.cmd: %.obj
	$(LINK86) $* '[$$sz]'

%.obj: %.a86
	$(RASM86) $< $$ pz sz

%.o: %.c
	$(CC) $(CFLAGS) $<
	$(STRIP) $@

clean:
	$(RM) *.o *.h86 *.log *.sym *.prn *.lst *.obj $(TOOLS) util.lib
	$(RM) dosver.exe $(EXTRAS)
	$(RM) cpmtest.img ccpmtest.img dostest.img hack.img
	(cd pce;make clean)


dostest.img: binaries Makefile test.txt env.dat
	(cd pce;make binaries)
	cp dosbase.img dostest.img
	-for i in $(PCETOOLS) $(TOOLS) $(EXTRAS);do \
	    mcopy -o -i dostest.img $$i ::`basename $$i|tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ` ; \
    done
	mcopy -o -i dostest.img test.txt ::TEST.TXT
	mcopy -o -i dostest.img env.dat ::ENV.DAT
	mdir -w -i dostest.img ::*.*

ccpmtest.img: binaries cpmtest.img startup.0
	(cd pce;make binaries)
	cp cpmtest.img ccpmtest.img
	cpmcp -f ibmpc-514ss ccpmtest.img startup.0 0:

cpmtest.img: binaries Makefile test.txt env.dat
	(cd pce;make binaries)
	cp cpmbase.img cpmtest.img
	cpmcp -f ibmpc-514ss cpmtest.img $(PCETOOLS) 0:
	cpmcp -f ibmpc-514ss cpmtest.img $(TOOLS) 0:
	cpmcp -f ibmpc-514ss cpmtest.img $(EXTRAS) 0:
	cpmcp -f ibmpc-514ss cpmtest.img test.txt 0:
	cpmcp -f ibmpc-514ss cpmtest.img env.dat 0:
	cpmls -F -f ibmpc-514ss cpmtest.img 0:*.*

test: cpmtest

ccpmtest: ccpmtest.img
	@./ccpm86

cpmtest: cpmtest.img
	@./cpm86

dostest: dostest.img
	@./dos
