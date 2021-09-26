LINK86=pcdev_linkcmd
RASM86=pcdev_rasm86

TOOLS=pceexit.cmd pcever.cmd pcemnt.cmd pcetime.cmd pceinit.cmd

all: binaries

binaries: $(TOOLS)

pceinit.obj: pceinit.a86 pce.a86 pcelib.a86 baselib.a86

pcetime.obj: pcetime.a86 pce.a86 pcelib.a86 baselib.a86

pcemnt.obj: pcemnt.a86 pce.a86 pcelib.a86 baselib.a86

pceexit.obj: pceexit.a86 pce.a86 pcelib.a86 baselib.a86

pcever.obj: pcever.a86 pce.a86 pcelib.a86 baselib.a86

baselib.a86: ../baselib.a86
	cp $< $@

%.cmd: %.obj
	$(LINK86) $* '[$$sz]'

%.obj: %.a86
	$(RASM86) $< $$ pz sz

clean:
	$(RM) *.o *.h86 *.sym *.prn *.lst *.obj $(TOOLS)
	$(RM) baselib.a86
