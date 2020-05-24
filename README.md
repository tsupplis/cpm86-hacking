# CP/M-86 hacking

A couple of experiments for fun with CP/M-86 including a quick and dirty port of
pce emulator tools to CP/M-86 
- pcetime
- pceexit
- pcever
- pcemnt
- pceinit (a combination of clear screan pcetime and pcever as CP/M-86
  start up command

All the tools are working on
- CP/M-86 1.1
- Concurrent CP/M 3.1
- Concurrent DOS 3.2 to 6.21
- Personal CP/M 2.04
- DOS Plus 1.2


## Tools used
- CP/M-86 1.1 for IBM PC XT. I recompile cpm.sys from patched sources from (http://www.cpm.z80.de/source.html)
  - CP/M-86 1.1, CCP/M-86 3.1 and PCP/M-2.0 can be found on (http://www.cpm.z80.de)
  - DOS Plus 1.2 and Patched kernel can be found on (https://www.seasip.info/Cpm/dosplus.html)
- The Excellent PCE emulator (http://www.hampa.ch/pce/pce-ibmpc.html)
- rasm86/linkcmd, DOS version from Digital Research (http://www.cpm.z80.de/binary.html)
- The Super Cool emu2 DOS emulator to run the DR tools on macOS and Linux (https://github.com/dmsc/emu2)

## PCE tools shortcuts
- pcetime has no option, It displays and sets up the clock using
    - BDOS Function if BDOS >= 3.0 (Dates from 01/01/1978)
    - Using System Variable Control Block on BDOS= 2.2
- pcemnt needs to accomodate CCP uppercasing so an toggle character '^' is used to this effect:
    - /Mnt^/User/^JOhn/^test.img becomes /mnt/USERS/john/TEST.IMG

![CP/M-86 1.1](images/cpm86.png)

## To dos
- pcetime to adjust seconds on BDOS >=3.0?
- move to microsoft/IBM MIT licensed Link/MASM
- provide DOS batch and CP/M submit for the build
- add a TOD replacement for CP/M-86
- study recompilation and patch of Date, Show and Dir and in the meantime regress test
- a clean reusable makefile
  CDOS 4.1 tools utilities on CCPM-86 2.0/CDOS 3.2
- clean up multi file and duplicates
- more tools accumulations

