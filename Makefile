AS=aztec42_as
CC=aztec42_cc
AR=aztec42_lib
STRIP=aztec42_sqz
ASM86=cpm86_asm86
GENCMD=cpm86_gencmd
MASM=pcdev_masm
LINK=pcdev_link
EXE2BIN=pcdev_exe2bin
CPM86_CFLAGS=-I. -B +0 
CPM86_CFLAGS=-I. -B +0 -D__CPM86__
CPM86_LDFLAGS=-lc86
DOS_CFLAGS=-I. -B +0 -D__DOS__
DOS_LDFLAGS=-lc
DOS11_LDFLAGS=-ld11
LD=aztec34_link
LINK86=pcdev_linkcmd
RASM86=pcdev_rasm86


CPM86TOOLS=rm.cmd more.cmd write.cmd dump.cmd mode.cmd ls.cmd \
    cls.cmd pause.cmd reboot.cmd tod.cmd ver.cmd touch.cmd wc.cmd \
    atinit.cmd attime.cmd ciotest.cmd ball.cmd getch.cmd \
    printenv.cmd mem.cmd zpdump.cmd
DOSTOOLS=dosver.com dosenv.com pspdump.com dosmem.com dosmem11.com \
    dosgetch.com
EXTRAS=clsansi.cmd rtctime.cmd rtcinit.cmd
PCETOOLS=pce/pceexit.cmd pce/pcever.cmd pce/pcemnt.cmd pce/pcetime.cmd \
    pce/pceinit.cmd

all: binaries

binaries: $(CPM86TOOLS) $(DOSTOOLS) $(EXTRAS)
	(cd pce;make binaries)

dist: cpm86-bin.zip pce-bin.zip dos-bin.zip hack.img

dos-bin.zip cpm86-bin.zip pce-bin.zip: binaries
	rm -f pce-bin.zip
	zip pce-bin.zip $(PCETOOLS) 
	rm -f cpm86-bin.zip
	zip cpm86-bin.zip $(CPM86TOOLS) 
	rm -f dos-bin.zip
	zip dos-bin.zip $(DOSTOOLS) 

hack.img: cpmtest.img
	cp $< $@

clsansi.cmd: clsansi.h86
	$(GENCMD) $< 8080

clsansi.h86: clsansi.a86
	$(ASM86) $< 

cls.cmd: cls.h86
	$(GENCMD) $< 8080

cls.h86: cls.a86
	$(ASM86) $< 

wc.cmd: wc.o util.lib
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

printenv.cmd: printenv.o util.lib
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

dosgetch.com: dosgetch.o
	$(LD) -o $@ $^ $(DOS11_LDFLAGS)

dosgetch.o: getch.c
	$(CC) $(DOS_CFLAGS) -o $@ $<
	$(STRIP) $@

getch.cmd: getch.o
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

ls.cmd: ls.o util.lib
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

ciotest.cmd: ciotest.o util.lib
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

cp.cmd: cp.o util.lib
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

touch.cmd: touch.o
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

rm.cmd: rm.o util.lib
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

write.cmd: write.o
	$(LD) -o $@ $< $(CPM86_LDFLAGS)

dump.cmd: dump.o
	$(LD) -o $@ $< $(CPM86_LDFLAGS)

ball.cmd: ball.o util.lib
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

mode.cmd: mode.o util.lib
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

more.cmd: more.o
	$(LD) -o $@ $< $(CPM86_LDFLAGS)

util.lib: util.o conio.o dirent.o dpb.o debug.o os.o gfx.o
	rm -f $@
	$(AR) $@ $^

gfx.o: gfx.asm
	$(AS) $<
	$(STRIP) $@

os.o: os.asm
	$(AS) $<
	$(STRIP) $@

zpdump.cmd: zpdump.o
	$(LD) -o $@ $< $(CPM86_LDFLAGS)

zpdump.o: pspdump.c
	$(CC) $(CPM86_CFLAGS) -o $@ $<
	$(STRIP) $@

pspdump.com: pspdump.o
	$(LD) -o $@ $< $(DOS11_LDFLAGS)

pspdump.o: pspdump.c
	$(CC) $(DOS_CFLAGS) -o $@ $<
	$(STRIP) $@

dosenv.com: dosenv.o
	$(LD) -o $@ $< $(DOS_LDFLAGS)

dosenv.o: dosenv.c
	$(CC) $(DOS_CFLAGS) $<
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

dosmem.com: dosmem.exe
	$(EXE2BIN) dosmem.exe dosmem.com

dosmem.exe: dosmem.obj
	$(LINK) dosmem \;

dosmem.obj: dosmem.asm
	$(MASM) dosmem \;

dosmem11.com: dosmem11.exe
	$(EXE2BIN) dosmem11.exe dosmem11.com

dosmem11.exe: dosmem11.obj
	$(LINK) dosmem11 \;

dosmem11.obj: dosmem11.asm
	$(MASM) dosmem11 \;

dosver.com: dosver.exe
	$(EXE2BIN) dosver.exe dosver.com

dosver.exe: dosver.obj
	$(LINK) dosver \;

dosver.obj: dosver.asm
	$(MASM) dosver \;

%.cmd: %.obj
	$(LINK86) $* '[$$sz]'
	[ -f "$@" ]

%.obj: %.a86
	$(RASM86) $< $$ pz sz
	[ -f "$@" ]

%.o: %.c
	$(CC) $(CPM86_CFLAGS) $<
	$(STRIP) $@
	[ -f "$@" ]

clean:
	$(RM) *.o *.h86 *.log *.sym *.prn *.lst *.obj $(CPM86TOOLS) $(DOSTOOLS) util.lib
	$(RM) dosver.exe $(EXTRAS)
	$(RM) cpmtest.img ccpmtest.img dostest.img hack.img
	(cd pce;make clean)


dostest.img: binaries Makefile test.txt env.dat
	(cd pce;make binaries)
	cp dosbase.img dostest.img
	-for i in $(PCETOOLS) $(DOSTOOLS) $(EXTRAS);do \
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
	cpmcp -f ibmpc-514ss cpmtest.img $(CPM86TOOLS) 0:
	cpmcp -f ibmpc-514ss cpmtest.img $(EXTRAS) 0:
	cpmcp -f ibmpc-514ss cpmtest.img test.txt 0:
	cpmcp -f ibmpc-514ss cpmtest.img env.dat 0:
	cpmls -F -f ibmpc-514ss cpmtest.img 0:*.*

test: cpmtest

ccpmtest: ccpmtest.img
	@./ccpm86

cpmtest: cpmtest.img
	@./cpm86

dosplus: cpmtest.img dostest.img
	@./dosplus

cdos41test: cpmtest.img dostest.img
	@./cdos41

dostest: dostest.img
	@./dos
