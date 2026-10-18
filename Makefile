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
LD=aztec42_link
LINK86=pcdev_linkcmd
RASM86=pcdev_rasm86


CPM86TOOLS=rm.cmd more.cmd copycon.cmd dump.cmd mode.cmd ls.cmd \
    cls.cmd pause.cmd reboot.cmd tod.cmd ver.cmd touch.cmd wc.cmd \
    atinit.cmd attime.cmd ciotest.cmd ball.cmd ballc.cmd getch.cmd \
    printenv.cmd mem.cmd zpdump.cmd wait.cmd prepoch.cmd sine.cmd
DOSTOOLS=dosver.com dosenv.com pspdump.com dosmem.com \
    dosgetch.com
EXTRAS=clsansi.cmd rtctime.cmd rtcinit.cmd
PCETOOLS=pce/pceexit.cmd pce/pcever.cmd pce/pcemnt.cmd pce/pcetime.cmd \
    pce/pceinit.cmd

all: binaries

binaries: $(CPM86TOOLS) $(DOSTOOLS) $(EXTRAS)
	(cd pce;make binaries)

dist: cpm86-lib.zip cpm86-bin.zip pce-bin.zip dos-bin.zip disks.zip

disks.zip: cpm86-bin.img dos-bin.img
	zip disks.zip cpm86-bin.img dos-bin.img

cpm86-lib.zip: util.lib
	rm -f cpm86-lib.zip
	zip cpm86-lib.zip util.lib *.h

dos-bin.zip cpm86-bin.zip pce-bin.zip: binaries
	rm -f pce-bin.zip
	zip pce-bin.zip $(PCETOOLS) 
	rm -f cpm86-bin.zip
	zip cpm86-bin.zip $(CPM86TOOLS) 
	rm -f dos-bin.zip
	zip dos-bin.zip $(DOSTOOLS) 

dos-bin.img: dostest.img
	cp $< $@

cpm86-bin.img: cpmtest.img
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

prepoch.cmd: prepoch.o util.lib
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

copycon.cmd: copycon.o
	$(LD) -o $@ $< $(CPM86_LDFLAGS)

dump.cmd: dump.o
	$(LD) -o $@ $< $(CPM86_LDFLAGS)

ballc.cmd: ballc.o util.lib
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

ballc.o: ballc.c
	$(CC) $(CPM86_CFLAGS) -o $@ $<
	$(STRIP) $@

sine.cmd: sine.o util.lib
	$(LD) -o $@ $^ -lm $(CPM86_LDFLAGS)

sine.o: sine.c
	$(CC) $(CPM86_CFLAGS) -o $@ $<
	$(STRIP) $@
	
ball.cmd: ball.h86
	$(GENCMD) $< 8080

ball.h86: ball.a86
	$(ASM86) $<

wait.cmd: wait.h86
	$(GENCMD) $< 8080

wait.h86: wait.a86
	$(ASM86) $<

mode.cmd: mode.o util.lib
	$(LD) -o $@ $^ $(CPM86_LDFLAGS)

more.cmd: more.o
	$(LD) -o $@ $< $(CPM86_LDFLAGS)

util.lib: util.o conio.o dirent.o dpb.o debug.o time.o os.o gfx.o sub.o subfile.o
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

dosenv.com: dosenv.exe
	$(EXE2BIN) dosenv.exe dosenv.com

dosenv.exe: dosenv.obj
	$(LINK) dosenv \;

dosenv.obj: dosenv.asm
	$(MASM) dosenv \;

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
	$(RM) cpmtest.img ccpmtest.img cpm86-bin.img dostest.img dos-bin.img
	(cd pce;make clean)


dostest.img: binaries Makefile 
	(cd pce;make binaries)
	cp dosbase.img dostest.img
	-for i in $(DOSTOOLS) $(CPM86TOOLS);do \
	    mcopy -o -i dostest.img $$i ::`basename $$i|tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ` ; \
    done
	mdir -w -i dostest.img ::*.*

ccpmtest.img: binaries cpmtest.img startup.0
	(cd pce;make binaries)
	cp cpmtest.img $@
	rm -f tmp.img
	python3 tools/cpm86twist.py untwist $@ tmp.img
	cpmcp -f cpm86-320 tmp.img startup.0 0:
	python3 tools/cpm86twist.py twist tmp.img $@
	rm -f tmp.img

cpmtest.img: binaries Makefile test.txt env.dat
	(cd pce;make binaries)
	cp cpmbase.img $@
	rm -f tmp.img
	python3 tools/cpm86twist.py untwist $@ tmp.img
	cpmcp -f cpm86-320 tmp.img $(PCETOOLS) 0:
	cpmcp -f cpm86-320 tmp.img $(CPM86TOOLS) 0:
	cpmcp -f cpm86-320 tmp.img $(EXTRAS) 0:
	cpmcp -f cpm86-320 tmp.img test.txt 0:
	cpmcp -f cpm86-320 tmp.img *.h *.lib 0:
	cpmcp -f cpm86-320 tmp.img env.dat 0:
	cpmls -F -f cpm86-320 tmp.img 0:*.*
	python3 tools/cpm86twist.py twist tmp.img $@
	rm -f tmp.img

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
