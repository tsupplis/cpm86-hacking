#using local wrappers
LD=pcdev_linkcmd
AS=pcdev_rasm86
#emu2 usage with DR tools
#LD=emu2 linkcmd.exe
#AS=emu2 rasm86.exe
CMDS=pceexit.cmd pcever.cmd pcemnt.cmd pcetime.cmd \
  pceinit.cmd cls.cmd pause.cmd ver.cmd tod.cmd  \
  attime.cmd atinit.cmd

all: binaries

binaries: $(CMDS)

images: cpmtest.img dostest.img cpm86at.img

dist: hacks-bin.lha

hacks-bin.lha: binaries
	rm -f hacks-bin.lha
	lha a hacks-bin.lha $(CMDS) 

cpm86at.img: binaries cpm86at-org.img
	cp cpm86at-org.img cpm86at.img
	cpmcp -f cpm86-144feat cpm86at.img $(CMDS) 0:
	cpmcp -f cpm86-144feat cpm86at.img at.sub 0:autoexec.sub
	cpmls -F -f cpm86-144feat cpm86at.img 0:*.*

cpmtest.img: binaries
	cpmrm -f ibmpc-514ss cpmtest.img 0:*.*
	cpmcp -f ibmpc-514ss cpmtest.img $(CMDS) 0:
	cpmls -F -f ibmpc-514ss cpmtest.img 0:*.*

dostest.img: binaries
	-mdel -i cdostest.img ::*.CMD
	mcopy -o -i cdostest.img pcemnt.cmd ::PCEMNT.CMD
	mcopy -o -i cdostest.img pcever.cmd ::PCEVER.CMD
	mcopy -o -i cdostest.img pceexit.cmd ::PCEEXIT.CMD
	mcopy -o -i cdostest.img pceinit.cmd ::PCEINIT.CMD
	mcopy -o -i cdostest.img pcetime.cmd ::PCETIME.CMD
	mcopy -o -i cdostest.img attime.cmd ::ATTIME.CMD
	mcopy -o -i cdostest.img atinit.cmd ::ATINIT.CMD
	mcopy -o -i cdostest.img cls.cmd ::CLS.CMD
	mcopy -o -i cdostest.img ver.cmd ::VER.CMD
	mcopy -o -i cdostest.img pause.cmd ::PAUSE.CMD
	mdir -w -i cdostest.img ::*.CMD

%.cmd: %.obj
	$(LD) '$*[$$sz]'

pceinit.obj: pceinit.asm pce.asm pcelib.asm
	$(AS) $< \$$ pz sz

ver.obj: ver.asm pce.asm pcelib.asm
	$(AS) $< \$$ pz sz

atinit.obj: atinit.asm pce.asm pcelib.asm
	$(AS) $< \$$ pz sz

attime.obj: attime.asm pce.asm pcelib.asm
	$(AS) $< \$$ pz sz

pcetime.obj: pcetime.asm pce.asm pcelib.asm
	$(AS) $< \$$ pz sz

tod.obj: tod.asm pce.asm pcelib.asm
	$(AS) $< \$$ pz sz

pcemnt.obj: pcemnt.asm pce.asm pcelib.asm
	$(AS) $< \$$ pz sz

pceexit.obj: pceexit.asm pce.asm pcelib.asm
	$(AS) $< \$$ pz sz

pcever.obj: pcever.asm pce.asm pcelib.asm
	$(AS) $< \$$ pz sz

cls.obj: cls.asm
	$(AS) $< \$$ pz sz

pause.obj: pause.asm
	$(AS) $< \$$ pz sz

clean:
	$(RM) *.obj *.sym *.lst
	$(RM) $(CMDS)

test: cpmtest.img
	./cpm86
