# CP/M-86 Hacking

## Synpsis

A couple of experiments for fun with CP/M-86. The focus is on IBM XT CP/M-86 and derivatives on PC. But most of the tools work with the Just4Fun V20-MBC SBC.

## Tools 

- LS: Directory listing
```
INF: Usage: ls [-h] | [-a] [-p] [-s|-r] [-l|-b] filepat
INF: where filepat is [user/]filespec with wildcard
INF:     -h for help
INF:     -a for all files (including system)
INF:     -p for for pausing one screen at a time
INF:     -s for alphabetic sorting
INF:     -r for reverse alphabetic sorting
INF:     -l for long listing
INF:     -b for basic listing
```
- RM: File delete
```
INF: Usage: rm [-h] | filepat
INF: where filepat is [user/]filespec with wildcard
INF:     -h for help
```
- MORE: File content list with pause
```
INF: Usage: more -h | [infile]
INF: where infile is [user/]filespec
INF:     -h for help
```
- WRITE: Equivalent of cat>file
```
INF: Usage: write -h | [-a] filename
INF: where filename is [user/]filespec
INF:     -h for help
INF:     -a to happen to existing file
```
(Ctrl-C or Ctrl-Z used to finish input)
- DUMP: Hexadecimal dump
```
INF: Usage: dump -h | [-p][-r] [infile]
INF: where infile is [user/]filespec
INF:     -h for help
INF:     -p pausing every page
INF:     -r raw output
```
- MODE: Screen setup (CP/M-80 1.1 for PC/XT only)
```
INF: Usage: mode -h | option option ...
INF:     -h for help
INF: and options:
INF:      cls        Clear screen
INF:      cursor=on  Show cursor
INF:      cursor=off Hide cursor
INF:      line=on    Show status line
INF:      line=off   Hide status line
INF:      status=    Set status line message 
          (\s: space, \\: \, \u:upper, \l:lower)
INF:      col=40     Switch to 40 columns
INF:      col=80     Switch to 80 columns
INF:      mono       Switch to Mono
INF:      color      Switch to Color
INF:      fg=        Set foreground color (1-F)
INF:      bg=        Set background color (1-F)
INF:      scrdef     Default screen settings
```
- REBOOT: Simple cold or warm reboot (PC only)
- CLS: Clear screen (clsansi is a vt100/ansi version as opposed to vt52 for PC)
- PAUSE: submit tool waiting for a keystroke
- TOD: Replacement for CP/M-86 without the 78-99 year constraint and date/time validation including leap years. It does not fix the visual issue of the century hard coded to 19. Patches exist for that. It has exactly the same behaviour as the original CP/M-86 tod.cmd tool. (CP/M-86 1.1 for PC/XT only)
- VER: Displays the BDOS version
- BALL: A simple CGA demo
- GETCH: A simple keyboard scanner
- AT clock tools
    - ATTIME: Sync up clock (CP/M-86 1.1 for PC/XT with an AT compatible clock only)
    - ATINIT: Sync up clock and display boot banner (CP/M-86 1.1 for PC/XT with an AT compatible clock only)
- PCE tools (PCE Emulator only)
    - PCETIME: Sync up clock (CP/M-86 1.1 for PC/XT only)
    - PCEINIT: Sync up clock and display boot banner (CP/M-86 1.1 for PC/XT only)
    - PCEVER: Displays the Emulator version 
    - PCEEXIT: Leaves the Emulator
    - PCEMNT: Mount a disk image from the host

ATINIT and PCEINIT tools provide a quick configuration dump equivalent to what CP/M-86 1.1 displays on boot

```
CP/M-86 Version 2.2
Hardware Configuration: 
 - Memory: 640Kb
 - Math Coprocessor: No
 - Floppy Drive(s): 2
 - Hard Disk(s): 1
Date now: 2021-9-27 22:17:39.0
```

All the C tools (rm, ls, mode, more, write, dump) benefit from the file specification pattern of Aztec C: [user]/[drive]:[filespec]. as well basic \<file and \>file redirects are supported.

## What CP/M-86?

Unless mentioned otherwise, all the tools are working on 
- CP/M-86 1.1 
- Concurrent CP/M 3.1
- Concurrent DOS 3.2 to 6.21
- Personal CP/M 2.04
- DOS Plus 1.2

Those OSes work incredibly well on PCE on floppy and HD images. This emulator is simple, small and works a treat. Fantastic...

![CP/M-86 1.1](images/cpm86.png)

![CP/M-86 1.1 Apps](images/cpmapps.png)

## PCE tools shortcuts

- pcetime has no option, It displays and sets up the clock using
    - BDOS Function 68h (T_SET) if BDOS >= 3.0 (Dates from 01/01/1978)
    - Using System Variable Control Block through BDOS Function 31h (S_SYSVAR) on BDOS= 2.2
- pcemnt needs to accomodate CCP uppercasing so an toggle character '^' is used to this effect:
    - /Mnt^/User/^JOhn/^test.img becomes /mnt/USERS/john/TEST.IMG

## TODOs

- cp equivalent
- configuration information in pceinit as in CP/M-86 bdos/bios init screen
- attime to adjust seconds on BDOS >=3.0? T_SET resets the seconds to 0 on invokation
- study recompilation and patch of Date, Show and Dir and in the meantime regress test
- clean up code duplicates
- more tools/experiments
- sort out why cpmtools refuses to work with 320kb (for testing I use single face 160K
  images). ibmpc-514ds does not seem to work
- explore further CP/M-86 Big Brothers (PCP/M 2.04 and CCP/M 3.1). At first glance, they
are giving you better CP/M-Plus like experience than the rough CP/M 2.2 interface of CP/M-86 but CCP/M-86 has a lot of quirks and PCP/M Keyboard configuration is a bit tricky (Fully IBM PC compatible?) .... to be explored
- deliver a guide to rebuild CP/M-86 from its sources with all the patches, AT support and 2020 look and feel. It works well but publishing the fixes and the code may not be allowed. all sources of information and code to start were found on (http://www.cpm.z80.de/source.html). It just required a lot of effort to compare/consolidate code, annotations, patches and comparison with existing binary versions. A lot of fun.
- try to pull/port a version of Emacs or VI on CP/M-86. I could not find one working out of the box (I am working on figuring out an elvis (https://github.com/udo-munk/stevie) and femacs in assembler ....


## Build Environment
- Makefile for DOS Aztec C cross compilers targetting CP/M-86.  (May require adaptation. emu2 is used to run dos compiler on unix/mac)
- aztec c compiler version 3.4
- rasm86/linkcmd, DOS version from Digital Research (http://www.cpm.z80.de/binary.html)

```
LINK86 Linkage Editor  02/Feb/87      Version 2.02
Cross Linker: DOS -> CPM-86 & CDOS-286      1/2/86
```
```
RASM-86 Assembler   12-Mar-87  PC-DOS Version 1.4a
```
- The Super Cool emu2 DOS emulator to run the DR tools on macOS and Linux (https://github.com/dmsc/emu2). This is an incredible way to bring dos command line development tools to a modern and up to date shell/make/whatever based dev environment. Another stunning emulator. Emu2 and PCE are an incredible pair.

For a rudimentary, cross development environment, look at (https://github.com/tsupplis/cpm86-crossdev).

## Test Environment
- CP/M-86 1.1 for IBM PC XT. I recompile cpm.sys from patched sources from (http://www.cpm.z80.de/source.html). The simple way to start however is probably (http://www.cpm.z80.de/download/144cpm86.zip)
  - CP/M-86 1.1, CCP/M-86 3.1 and PCP/M-2.0 can be found on (http://www.cpm.z80.de)
  - DOS Plus 1.2 and Patched kernel can be found on (https://www.seasip.info/Cpm/dosplus.html)
- The Excellent PCE emulator (http://www.hampa.ch/pce/pce-ibmpc.html)
- mtools 4 and cpmtools 2.20

## Quick points on CP/M-86
Despite being a very primitive OS (in some cases actually enjoyably primitive...No time management at all outside of the clock, for example), it is possible to do quite an amount of things with a couple of good tools:
- DR CB86 2.0 (http://www.cpm.z80.de/binary.html)
- DR C86 1.32 (http://www.cpm.z80.de/binary.html)
- Aztec C 3.2 (Although Aztec C 3.4 or C 4.10 (ANSI) cross compilation from DOS is better) (https://www.aztecmuseum.ca/compilers.htm)
- Turbo Pascal 3.01A and Poly Pascal 3.1 (Turbo Pascal Sibling)
- RASM86 Macro Assembler (http://www.cpm.z80.de/binary.html)
- Microsoft Basic (http://www.retroarchive.org/cpm/lang/lang.htm) and Personal Basic Interpreters (http://www.cpm.z80.de/binary.html)
- PL/M-86 (Only cross compilation from DOS)

The big cool thing with this tiny OS is how small it is. I never configure more than 128K on my PCE VM and a single person can play as a hobbyist with it. As mention earlier it works also super well with the Just4Fun V20-MBC SBC (https://hackaday.io/project/170924-v20-mbc-a-v20-8088-8080-cpu-homebrew-computer). Cool other thing: it is Y2K compliant with TOD replacement above and a small visual hack. Yep no date management ...

It makes The early sources of MS-DOS as attractive. MS-DOS 2.X is far more flexible and has a bigger ecosystem than CP/M-86. This makes it more of a challenge ;-) 

DR tools are available through (http://www.cpm.z80.de/binary.html) and many other sources.
I found the last release of Turbo Pascal and Poly Pascal on a very exhaustive danish site focused on the RC700 series computers who are by the way also emulated by PCE. They are not PC compatible but both pascal are generic and work on the IBM PC based CP/M-86 family.
- (https://rc700.dk/software.php?name=RC750_TurboPascal_v3.01a)
- (https://rc700.dk/software.php?name=RC759_PolyPascal_v3.1) 

But the files need to be extracted from disk images using cpmtools using the following definition:

```
diskdef rc75x
  seclen 1024
  tracks 154
  sectrk 8
  blocksize 2048
  maxdir 512
  boottrk 4
  os 3
end
```

Turbo Pascal 3.01A is the last releasse on CP/M-86 (Don't forget to configure it using
tinst) and is in english. Poly pascal is also in english but the message file and the online help are in Danish. Not a big deal to translate them. Poly Pascal is really a twin, sibling or ancestor of Turbo Pascal. Kinda cool to play with it.

Macro assembler, C, Pascal, Structure Basic ... what else do we need?

Cross Compilation can be done either using DOS emulation (emu2, pce) or using a hybrid OS
- DOS Plus 1.2
- Concurrent DOS 6.21 XM

... run all the tools above (Both in their DOS and CP/M version) and you can test directly the cmd binaries.

A few CP/M-86 emulators for DOS exist (ame86.exe, cpm86.exe) unfortunately their sources
are not to be seen anywhere and they woulld need a bit of maintenance. only rudimentary
programs work.

Finally, Assembly using asm86 and gencmd can also be done but it needs to be done on CP/M environments (CP/M-80 or CPM-86 derivatives can be used)

It is a bit lacking on the tooling side though ... so I will port tools little by little for comfort sake.
- VE+ 2.03 (or better VE+ 2.33a) is really the only strong editor I found. (http://www.retroarchive.org/cpm/text/text.htm) (There is also TED a basic editor that allows you to stay away from ED). 
- Automation through submit is very very rudimentary (no dream of makefiles)
- There is no real solid CP/M-86 emulation as mentioned... But piggy backing the INT E0H API end point on emu2 perhaps? after all the 2 CP/M-86 and DOS 1.1 APIs are pretty much aligned...

Still it is a funny bit of discovery and archeology...
