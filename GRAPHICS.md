# Graphics compatibility and hardware limits

The library chooses BIOS or XIOS graphics from the BDOS version and OS type.
That choice affects which video adapters work. An EGA/VGA card's ability to run
CGA software through its BIOS does not make it compatible with every CGA
register sequence issued by an older XIOS.

This applies to `ballc.cmd`, `sine.cmd`, `rose.cmd` and programs using `gfx.c`/`gfx.asm`.
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
| `0Dh` | 320x200, 16 EGA colors | Issued to BIOS | Patched XIOS, capability bit 1 (colour EGA, 64 KB is enough, or VGA) |
| `0Eh` | 640x200, 16 EGA colors | Issued to BIOS | Patched XIOS, capability bit 1 |
| `0Fh` | 640x350, monochrome EGA | Issued to BIOS | Rejected |
| `10h` | 640x350, 16 EGA colors | Issued to BIOS | Patched CCP/M-86 3.1 and Concurrent DOS 4.1, capability bit 0 (EGA >=128 KB or VGA) |
| `11h` | 640x480, 2 colors | Issued to BIOS | Patched XIOS, capability bit 2 (VGA) |
| `12h` | 640x480, 16 colors | Issued to BIOS | Patched XIOS, capability bit 2 (VGA) |
| `13h` | 320x200, 256 colors | Issued to BIOS | Patched XIOS, capability bit 2 (VGA) |

A BIOS request still needs a video BIOS and adapter supporting that mode.
The library does not probe or verify every BIOS mode. On XIOS, unsupported
native EGA requests are rejected before hiding the status line or changing
the display. `sine.cmd` needs mode `10h` and `rose.cmd` mode `12h`: use the BIOS backend or either patched XIOS with a suitable EGA/VGA adapter.

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
established. The library does not assume that these releases support its native EGA/VGA extension; modes 0Dh..13h remain rejected there. Mode 0Fh remains unsupported through XIOS everywhere.

## Exception: patched ccpm86-kernel

The [ccpm86-kernel distribution](https://github.com/tsupplis/ccpm86-kernel)
with the EGA/VGA patch extends the 3.1 XIOS. See its
[patch notes](https://github.com/tsupplis/ccpm86-kernel/blob/egavga/PATCHES.md).
Use a kernel/image built with that patch and a rebuilt graphics demo.

| Operation | Legacy CGA | EGA/VGA detected at kernel startup |
| --- | --- | --- |
| Enter mode 4/6 | Existing CGA register setup | XIOS calls video BIOS with hardware IRQs temporarily masked |
| Enter native mode 10h | Rejected | Capability-gated BIOS setup on EGA with >=128 KB or VGA |
| Draw pixels | Packed CGA memory at `B800:0000` | CGA layout for modes 4/6; four planes at `A000:0000` for mode 10h |
| Change palette | CGA color-select port | XIOS translates the packed color selection into BIOS palette calls |
| Return to text | Existing CGA text setup | BIOS restores standard 25-row text/font; XIOS redraws saved console, cursor and status |
| Switch consoles in graphics | Existing behavior | Blocked until the owner returns to text |

The kernel preserves video BIOS service and font/parameter vectors,
including the IBM VGA ROM's `INT 6Dh` trampoline. BIOS calls use a private
kernel stack and restore the original interrupt mask and caller registers.

The palette extension uses the existing `ESC !` eight-byte register packet
to XIOS function 30, with `CH=2` and the packed CGA color-select byte in
`CL`. XIOS supplies the calling console number and checks graphics
ownership. The library sends it for Concurrent BDOS `31h` and `41h`, after the legacy CGA
port write. Original 3.1 XIOS rejects that function through the escape back
door without printing the packet. Releases 3.2 through 5.x and DOS Plus
retain their previous palette path.

This supports **CGA modes on EGA/VGA and native EGA mode 10h**.
It does not provide a general application `INT 10h` gateway. It restores
standard BIOS text modes, not arbitrary custom fonts, row counts or
application-defined video state. Finish the demo normally; recovery after
an abnormal exit is not guaranteed.

## Native EGA/VGA modes

| BIOS mode | Size | Colours | XIOS mode | Capability bit | Memory layout | Text font |
| --- | --- | --- | --- | --- | --- | --- |
| `0Dh` | 320x200 | 16 | 60h | 1 | Four planes, 40 bytes/line | 8x8, 40x25 |
| `0Eh` | 640x200 | 16 | 70h | 1 | Four planes, 80 bytes/line | 8x8, 80x25 |
| `10h` | 640x350 | 16 | 50h | 0 | Four planes, 80 bytes/line | 8x14, 80x25 |
| `11h` | 640x480 | 2 | 80h | 2 | Plane 0, 80 bytes/line | 8x16, 80x30 |
| `12h` | 640x480 | 16 | 90h | 2 | Four planes, 80 bytes/line | 8x16, 80x30 |
| `13h` | 320x200 | 256 | A0h | 2 | Chain-4, one byte per pixel | 8x8, 40x25 |

Mode 0Fh (EGA monochrome) remains BIOS-only. The patched kernel accepts
mode 10h only with an enhanced colour adapter and BIOS-reported video memory
of at least 128 KB; a 64 KB EGA cannot supply four full planes. Modes
0Dh/0Eh work on any colour EGA. Modes 11h..13h need a VGA identified by
INT 10h AX=1A00h, so a 256 KB EGA is not mistaken for VGA.

The library probes reserved XIOS v1 selector 30 using INT E6h, only on
Concurrent BDOS 31h or 41h. It requires AX=4745h and the requested mode's
BX bit: bit 0 for 10h, bit 1 for 0Dh/0Eh, bit 2 for 11h/12h/13h. Original
3.1 kernels treat this selector as a no-op; the probe supplies BX=0 and
rejects the mode without hiding the status line or touching video memory.
CGA/MDA and patched kernels without a suitable adapter also return no
native capability.

Mode entry is ESC a followed by the BIOS mode byte; XIOS translates it
into the native mode above. Text restoration and graphics ownership use the existing
enhanced-adapter path. No raw application BIOS mode call is needed.

Planar pixel drawing (0Dh..12h) uses EGA write mode 2: the CPU color's four bits expand into
four planes, while destination latches and a bit mask preserve adjacent
pixels. XOR uses the graphics controller's logical XOR operation. Each
register/latch transaction masks interrupts and restores the library's
normal write-mode-0, replace-operation and full-bit-mask defaults afterward.
The library owns these drawing registers while its graphics mode is active;
it does not preserve arbitrary register changes made by other drawing code.
No VGA-only register reads or write mode 3 are used. Mode 11h keeps
colour bit 0 (and the XOR bit) only, like CGA mode 6.

Mode 13h uses a separate backend: one byte per pixel at `A000:y*320+x`
in the BIOS-configured chain-4 layout. No graphics-controller register is
touched (its mode register keeps the 256-colour shift bit), and the full
colour byte is stored, so bit 7 does not request XOR.

Text uses the BIOS-selected ROM font pointer at vector 43h, with the cell
heights in the table above. It does not assume a font offset inside a
particular EGA ROM. Planar text writes each glyph line as two byte-wide
write-mode-2 operations (foreground, then black background).

For native EGA palette changes, call:

```c
gfx_palette_entry(12, 0x01);  /* map logical color 12 to dark blue */
gfx_palette_entry(12, 0x3c);  /* restore light red */
```

Index must be 0..15 and color 0..63. The raw six-bit EGA value uses primary
B/G/R in bits 0/1/2 and secondary B/G/R in bits 3/4/5. This is an attribute
palette value, not a VGA DAC RGB triple. Invalid arguments return -1 without
changing the display. On XIOS, function 30 CH=3 carries CL=index and DH=color;
the escape back door supplies DL=calling console. The kernel checks mode and
graphics ownership before issuing BIOS function 1000h. The same call applies to all planar
modes 0Dh..12h; mode 13h rejects it. Keep gfx_palette for CGA selection;
the XIOS EGA/VGA path ignores that CGA-only operation.

For VGA modes 11h..13h, `gfx_dac_entry(index, red, green, blue)` sets DAC
entry 0..255 with 6-bit components. The BIOS backend uses INT 10h
AX=1010h. On XIOS the library writes ports 3C8h/3C9h directly while it owns
graphics, as it already does for planar drawing; restoring text mode
reloads the BIOS default DAC. In planar VGA modes, attribute palette values
select DAC entries 0..63.

## Drawing and console behavior

| Detail | XIOS path |
| --- | --- |
| Pixel memory | CGA: packed pixels in alternating 8 KB banks at `B800:0000`. EGA/VGA planar: four planes at `A000:0000` (8,000..38,400 bytes each). Mode 13h: 64,000 bytes at `A000:0000`. |
| Bounds | Mode width x height from the tables above. Out-of-range pixels ignored. |
| XOR | Color bit 7 requests XOR, except mode 13h |
| Initial palette | Bright CGA colors |
| Palette interface | BIOS-style packed `BX`: high byte 0 for background/intensity (foreground in mode 6), 1 for palette selection |
| Text | Software 8x8 PC ROM font in CGA; BIOS-selected ROM font (vector 43h) in EGA/VGA modes. ASCII 0..127; other bytes become `?`. |
| Status line | Hidden for graphics; restored on return to text |
| Mode result | `0` means request issued; escape output provides no success acknowledgement |
| Concurrency | One graphics owner; no per-console graphics framebuffer save/restore |

Run one graphics application at a time, on the foreground console, and exit
before switching consoles. Direct framebuffer drawing is not synchronized
with arbitrary application BIOS calls.

## Verification and remaining limits

| Configuration | Evidence |
| --- | --- |
| Patched CCP/M-86 3.1, PCE EGA/VGA | CGA modes 4/6; native mode 10h with all 16 colors, palette entry changes, XOR/neighbour preservation, edge pixels, 8x14 text, SINE and repeated text restoration/console switching |
| Legacy CGA path | Existing direct-register path retained; CGA regression checked separately |
| Concurrent DOS 3.2 and 4.1 | User reported working XIOS graphics before this extension |
| Concurrent DOS 6.2 | Earlier user tests passed; current policy selects the BIOS backend |
| DOS Plus | crtreset tested in PCE VGA across three consecutive runs after scrolling and BIOS output; full XIOS graphics coverage still pending |
| Physical EGA/VGA and other video ROMs | Not yet validated |
| Patched CCP/M-86 3.1, PCE VGA | Modes 0Dh/0Eh/10h/11h/12h/13h: colour bars, edge clipping, XOR, ROM-font text, EGA palette entry and VGA DAC changes, text restoration; ROSE in mode 12h |
| Patched CCP/M-86 3.1, PCE EGA | Modes 0Dh/0Eh/10h as above; 11h/12h/13h correctly rejected before any display change |
| CP/M-86 1.1 BIOS backend, PCE VGA | ROSE in mode 12h |
| Native EGA on other XIOS versions | Not enabled; no compatibility claim for Concurrent 3.2..5.x or DOS Plus |

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
