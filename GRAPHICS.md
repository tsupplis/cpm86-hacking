# Graphics compatibility and hardware limits

The library chooses BIOS or XIOS graphics from the BDOS version and OS type.
That choice affects which video adapters work. An EGA/VGA card's ability to run
CGA software through its BIOS does not make it compatible with every CGA
register sequence issued by an older XIOS.

This applies to `ballc.cmd`, `sine.cmd` and programs using `gfx.c`/`gfx.asm`.
The separate `ball.cmd` assembly demo has its own graphics code; changes to the
C library do not change its behavior.

## OS selection

Versions below are hexadecimal BDOS versions reported by function 12,
not necessarily the version printed in a product banner. Concurrent means
`ostype() & OSTYPE_MULTIUSER` is nonzero.

| BDOS version | OS type | Backend |
| --- | --- | --- |
| `22h` | CP/M-86 family | BIOS |
| `31h..5Fh` | Concurrent | XIOS |
| `41h` | Not Concurrent; DOS Plus | XIOS |
| `60h` and later | Any, including Concurrent | BIOS |
| Other combinations | Any | Unsupported; `gfx_mode` returns `-1` |

The `60h` threshold takes precedence over the Concurrent test. Concurrent
DOS 6.x therefore uses BIOS mode, palette, pixel and text operations. This is
the selected policy following successful user testing of BIOS graphics on
that family. DOS Plus remains on XIOS pending further testing.

## Available modes

| Mode | Display | BIOS backend | XIOS backend |
| --- | --- | --- | --- |
| `03h` | 80-column text | Issued to BIOS | Issued to XIOS |
| `04h` | 320x200, four CGA colors | Issued to BIOS | Supported |
| `06h` | 640x200, two CGA colors | Issued to BIOS | Supported |
| `0Dh` | 320x200, 16 EGA colors | Issued to BIOS | Rejected |
| `0Eh` | 640x200, 16 EGA colors | Issued to BIOS | Rejected |
| `0Fh` | 640x350, monochrome EGA | Issued to BIOS | Rejected |
| `10h` | 640x350, 16 EGA colors | Issued to BIOS | Rejected |

A BIOS request still needs a video BIOS and adapter supporting that mode.
The library does not probe or verify every BIOS mode. On XIOS, unsupported
native EGA requests are rejected before hiding the status line or changing
the display. `sine.cmd` needs mode `10h`, so it currently needs the BIOS backend.

## Stock XIOS: match the adapter

The original CCP/M-86 3.1 color graphics handler writes CGA CRTC, mode and
color-select registers directly. EGA and VGA require additional sequencer,
graphics-controller, attribute-controller and font-plane setup. CGA register
writes alone do not establish a complete EGA/VGA graphics mode or restore
its text font.

| Configuration | Expected behavior with the original CGA XIOS path |
| --- | --- |
| Physical CGA or emulator configured as CGA | Matching hardware for modes 4/6 |
| EGA configured as EGA | CGA register programming is insufficient; do not assume compatibility |
| VGA configured as VGA | Corrupted graphics and incomplete text restoration reproduced in PCE and DOSBox |
| MDA/Hercules | Text only through this library; not a CGA graphics target |

For the stock 3.1 XIOS, use `./ccpm86 cga` with the project's PCE launcher.
Selecting `vga` and requesting a CGA resolution does not substitute for
selecting a CGA adapter.

Other Concurrent/DOS Plus releases contain their own XIOS implementations.
Do not assume their mode-setting or adapter support matches 3.1, or that
every BIOS-compatible adapter is supported. User tests confirmed the earlier
CGA path on Concurrent DOS 3.2 and 4.1; broader hardware coverage is not
established. Native EGA modes remain rejected by this library's XIOS backend
regardless of what a particular release might support internally.

## Exception: patched ccpm86-kernel

The [ccpm86-kernel distribution](https://github.com/tsupplis/ccpm86-kernel)
with the EGA/VGA patch extends the 3.1 XIOS. See its
[patch notes](https://github.com/tsupplis/ccpm86-kernel/blob/egavga/PATCHES.md).
Use a kernel/image built with that patch and a rebuilt graphics demo.

| Operation | Legacy CGA | EGA/VGA detected at kernel startup |
| --- | --- | --- |
| Enter mode 4/6 | Existing CGA register setup | XIOS calls video BIOS with hardware IRQs temporarily masked |
| Draw pixels | Packed CGA memory at `B800:0000` | Same BIOS-established CGA memory layout |
| Change palette | CGA color-select port | XIOS translates the packed color selection into BIOS palette calls |
| Return to text | Existing CGA text setup | BIOS restores standard 25-row text/font; XIOS redraws saved console, cursor and status |
| Switch consoles in graphics | Existing behavior | Blocked until the owner returns to text |

The kernel preserves video BIOS service and font/parameter vectors,
including the IBM VGA ROM's `INT 6Dh` trampoline. BIOS calls use a private
kernel stack and restore the original interrupt mask and caller registers.

The palette extension uses the existing `ESC !` eight-byte register packet
to XIOS function 30, with `CH=2` and the packed CGA color-select byte in
`CL`. XIOS supplies the calling console number and checks graphics
ownership. The library sends it only for BDOS `31h`, after the legacy CGA
port write. Original 3.1 XIOS rejects that function through the escape back
door without printing the packet. Releases 3.2 through 5.x and DOS Plus
retain their previous palette path.

This supports **CGA modes on EGA/VGA**, not native EGA planar drawing.
It does not provide a general application `INT 10h` gateway. It restores
standard BIOS text modes, not arbitrary custom fonts, row counts or
application-defined video state. Finish the demo normally; recovery after
an abnormal exit is not guaranteed.

## Drawing and console behavior

| Detail | XIOS path |
| --- | --- |
| Pixel memory | CGA packed pixels and alternating 8 KB scan-line banks at `B800:0000` |
| Bounds | 320x200 or 640x200; out-of-range pixels ignored |
| XOR | Color bit 7 requests XOR |
| Initial palette | Bright CGA colors |
| Palette interface | BIOS-style packed `BX`: high byte 0 for background/intensity (foreground in mode 6), 1 for palette selection |
| Text | Software 8x8 IBM PC ROM font; ASCII 0..127, other bytes become `?` |
| Status line | Hidden for graphics; restored on return to text |
| Mode result | `0` means request issued; escape output provides no success acknowledgement |
| Concurrency | One graphics owner; no per-console graphics framebuffer save/restore |

Run one graphics application at a time, on the foreground console, and exit
before switching consoles. Direct framebuffer drawing is not synchronized
with arbitrary application BIOS calls.

## Verification and remaining limits

| Configuration | Evidence |
| --- | --- |
| Patched CCP/M-86 3.1, PCE EGA/VGA | Mode 4 bright palettes, mode 6, repeated graphics/text transitions, text/status restoration and console switching after exit |
| Legacy CGA path | Existing direct-register path retained; CGA regression checked separately |
| Concurrent DOS 3.2 and 4.1 | User reported working XIOS graphics before this extension |
| Concurrent DOS 6.2 | Earlier user tests passed; current policy selects the BIOS backend |
| DOS Plus | crtreset tested in PCE VGA across three consecutive runs after scrolling and BIOS output; full XIOS graphics coverage still pending |
| Physical EGA/VGA and other video ROMs | Not yet validated |
| Native EGA on XIOS | Not implemented |

PCE's EGA cursor renderer currently displays the requested two-scan-line
cursor as one line because of its end-scan-line handling. That is separate
from graphics mode entry/restoration; the kernel retains the requested
hardware cursor shape.


## DOS Plus cursor reset after direct BIOS output

DOS Plus has a hooked video BIOS and maintains a separate hardware scrolling
origin. A clear/home escape alone does not reset all of that state.
After restoring text mode, call the conio function `crtreset()` to re-enter
the current text mode through the installed `INT 10h` vector. This lets the
DOS Plus hook reset its console state as the BIOS resets the display.

This behavior is restricted to non-Concurrent BDOS `41h`. It clears the
text screen, selects the mode's default page and homes the cursor; it does
not preserve screen contents, custom fonts or custom row counts. Modes
0..3 and 7 retain their mode number. In graphics modes the extra mode reset
is skipped. Other systems keep the existing attribute/cursor reset behavior.
The graphics library does not invoke this destructive reset automatically.


DOS Plus validation: a PCE VGA guest ran a probe three times consecutively.
Each run scrolled 32 lines, wrote through the BIOS, called crtreset and
printed a marker through BDOS. The marker stayed on row 0 and the CRTC
display origin stayed zero after every run. Host routing checks also confirm
that Concurrent 4.1 and other BDOS versions do not enter the new BIOS reset.
