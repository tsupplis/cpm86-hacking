# util.lib API Reference

`util.lib` is the standard runtime utility library for CP/M-86 tools and programs in this repository. It provides OS detection, low-level BDOS/BIOS/hardware access, console manipulation, directory traversal, CGA/EGA graphics, timekeeping, environment variable handling, and submit (`$$$.sub`) batch chaining.

---

## Table of Contents

1. [Operating System & Low-Level Services (`util.h`)](#1-operating-system--low-level-services-utilh)
2. [Environment & String Utilities (`util.h`)](#2-environment--string-utilities-utilh)
3. [Console I/O & Terminal Control (`conio.h`)](#3-console-io--terminal-control-conioh)
4. [Directory & Disk FCB Handling (`dirent.h`)](#4-directory--disk-fcb-handling-direnth)
5. [Graphics (`gfx.h`)](#5-graphics-gfxh)
6. [Date, Time & Epoch (`util.h`)](#6-date-time--epoch-utilh)
7. [Submit Files & Process Chaining (`sub.h`)](#7-submit-files--process-chaining-subh)
8. [Debug Utilities (`debug.h`)](#8-debug-utilities-debugh)

---

## 1. Operating System & Low-Level Services (`util.h`)

Implemented across [`os.asm`](os.asm:1) and [`dpb.c`](dpb.c:1).

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `osver` | `int osver(void)` | Returns the OS version number (BDOS fn 12 / `0Ch`, low byte in `AL`). E.g. `0x22` = CP/M-86 1.1 / CP/M 2.2, `0x30` = CP/M Plus, `0x31` = Personal CP/M, `0x41` = DOS Plus. |
| `ostype` | `int ostype(void)` | Returns the OS type byte (BDOS fn 12 / `0Ch`, high byte in `AH`). E.g. `0x00` = CP/M single-user, `0x14` = Concurrent CP/M-86. Returns `0x10` if `AH` is 0. Check against `OSTYPE_MULTIUSER` (`0x04`). |
| `getcurdrv` | `int getcurdrv(void)` | Returns current default drive via BDOS fn 25 / `19h` (0 = `A:`, 1 = `B:`, ...). |
| `setcurdrv` | `int setcurdrv(int drv)` | Selects default drive via BDOS fn 14 / `0Eh` (0 = `A:`, 1 = `B:`, ...). |
| `getccpdrv` | `int getccpdrv(void)` | Returns the drive the running command/transient was loaded from across CP/M-86 1.1 (sysdata `+17h`), CCP/M 3.x (SCB `+4Bh`), and DOS Plus (SCB `+86h`). Returns drive index (0 = `A:` … 15 = `P:`) or `0x0FFF` if unknown. |
| `bdosx` | `int bdosx(int cx, int dx, int *es, int *bx)` | Invokes extended BDOS call (`INT 0E0h`) with `CX` (function) and `DX` (parameter). Returns `AX & 0xFF`, and writes returned `ES` and `BX` into `*es` and `*bx`. |
| `delay` | `void delay(unsigned int ticks)` | Delays execution for `ticks` timer ticks. Uses BIOS `INT 1Ah` on CP/M-86 1.1, or BDOS fn 141 (`8Dh`) on multitasking OSes. |
| `bioskey` | `int bioskey(void)` | Reads key directly from ROM BIOS via `INT 16h` (`AH=00h`). Returns `AX` (`AH` = scan code, `AL` = ASCII character). |
| `pckey` | `int pckey(void)` | Direct PC keyboard hardware read from port `0x60`, followed by keyboard acknowledge and 8259 PIC EOI (`0x20`). Returns raw scan code. |
| `xstatline` | `void xstatline(int mode)` | Toggles status line display on DOS Plus and CCP/M via OS system call table. |
| `dpb_load` | `void dpb_load(int drive, dpb_t *dpb)` | Reads the Disk Parameter Block (DPB) for `drive` (0 = `A:`, etc.) via BDOS fn 31 (`1Fh`) into `dpb_t`. |
| `getftime` | `int getftime(char *fcb)` | BDOS fn 102 (`66h`, `F_TIMEDATE`): reads the date/time stamps and password mode of an open FCB. Stamps land in the FCB: create/access at `+18h`, update at `+1Ch`; password mode at `+0Ch`. Requires version >= `30h` (MP/M II, CP/M 3, DOS Plus ...). Returns 0-3 on success, `0xFF` on failure or if unsupported (e.g. CP/M-86 1.1). |
| `setftime` | `int setftime(char *fcb, char *stamps)` | BDOS fn 116 (`74h`): sets the file date/time stamps. CP/M-86 v4 / DOS Plus only (version >= `41h`). `stamps` is 8 bytes (create/access stamp, then update stamp), each `DW` days since 1/1/78, `DB` BCD hour, `DB` BCD minute. The DMA address is pointed at `stamps` for the call and restored afterwards. Returns 0-3 on success, `0xFF` on failure or if unsupported. |
| `getpcode` | `unsigned int getpcode(void)` | BDOS fn 108 (`6Ch`, `P_CODE`): returns the program return code. CP/M 3 and above (version >= `30h`, not MP/M II). Returns `0xFFFF` if unsupported. |
| `setpcode` | `unsigned int setpcode(unsigned int code)` | BDOS fn 108 (`6Ch`): sets the program return code and returns it. `0x0000`-`0xFEFF` = no fatal error, `0xFF00`-`0xFF7F` = fatal error (a following `:` command is not run), `0xFFFD` = hardware error, `0xFFFE` = Control-C. `setpcode(0xFFFF)` is a read. Returns `0xFFFF` if unsupported. |
| `getcmode` | `unsigned int getcmode(void)` | BDOS fn 109 (`6Dh`, `C_MODE`): returns the console mode bitmap. CP/M 3 and above (version >= `30h`, not MP/M II). CP/M 3 bits: 0 fn 11 detects only ^C, 1 ^S does not pause, 2 no tab expansion / no ^P echo, 3 ^C does not terminate, 4 like bit 2, 7 CCP/M ^O not a byte bucket, 8-9 GET/SUBMIT behaviour of fn 11 (0 conditional, 1 always true, 2 always false, 3 input from keyboard). Personal CP/M uses bit 4 only. Returns `0xFFFF` if unsupported. |
| `setcmode` | `unsigned int setcmode(unsigned int mode)` | BDOS fn 109: sets the console mode bitmap and returns it (`setcmode(0xFFFF)` is a read). Returns `0xFFFF` if unsupported. |
| `p_cli` | `int p_cli(char *cmd)` | BDOS fn 150 (`96h`, `P_CLI`): sends a command line (max 127 chars) to the CLI, i.e. a resident system process or a program load. MP/M and Concurrent CP/M only (system type bit 0 or 2 set). Returns 0 on success, `0xFF` on failure or if unsupported. |
| `p_priority` | `int p_priority(int priority)` | BDOS fn 145 (`91h`, `P_PRIORITY`): sets the process priority; `0` is the highest, `0xFF` the lowest, transient programs start at 200. MP/M and Concurrent CP/M only (system type bit 0 or 2 set). Returns 0, or -1 if unsupported. |
| `conattach` | `int conattach(void)` | BDOS fn 146 (`92h`, `C_ATTACH`): blocks until the process's default console is available, then attaches to it. Done automatically by any console I/O while detached. MP/M and Concurrent CP/M only. Returns 0, or -1 if unsupported. |
| `condetach` | `int condetach(void)` | BDOS fn 147 (`93h`, `C_DETACH`): disconnects the program from screen and keyboard so that it runs in the background. MP/M, Concurrent CP/M and CP/M-86 v4 (version >= `41h`). Under CP/M-86 v4 using the keyboard while detached terminates the program, and only `INT 0E0h` calls may be used (no `INT 2nh` DOS-like calls); under MP/M and CCP/M screen output re-attaches (fn 146). Returns 0, or -1 if unsupported. |
| `getpid` | `unsigned int getpid(void)` | Process ID: the address (offset in the system data segment) of this process's descriptor (PD), from BDOS fn 156 (`9Ch`, `P_PDADR`, returned in `ES:BX`). MP/M and Concurrent CP/M only. Returns `0xFFFF` if unsupported (never a valid descriptor address). Unsigned, because addresses above `0x7FFF` are valid: compare PIDs for equality and print them with `%x`/`%u`. |
| `getppid` | `unsigned int getppid(void)` | Process ID of the parent process (PD field `p_parent`, offset `1Eh`). Returns 0 if the process has no parent, `0xFFFF` if unsupported. |
| `getpriority` | `int getpriority(void)` | Priority of this process (PD field `p_prior`, offset `05h`): 0 = highest ... 255 = lowest, 200 = normal transient. Returns -1 if unsupported. |
| `getppriority` | `int getppriority(void)` | Priority of the parent process, read from the parent's PD (the parent PD lies in the same segment). Returns -1 if there is no parent or if unsupported. Typical use, as in `submit.a86`: `p_priority(getppriority() - 1)` to run just better than the parent. |

> Already provided by the Aztec C base library, so not part of `util.lib`: `getusr()` / `setusr(n)` / `rstusr()` (BDOS fn 32, get/set user number), `fcbinit(name, fcb)` (file name to FCB) and `bdos(fn, dx)`. They are documented in [section 4](#aztec-c-base-library-companions-fcbinit-getusr-setusr-rstusr) because the FCB-based functions here (`getftime`, `setftime`, `dirent_*`) work on FCBs built that way. `p_chain` (BDOS fn 47) is documented in [section 7](#7-submit-files--process-chaining-subh).
>
> `getftime`, `setftime`, `getpcode`, `setpcode`, `getcmode`, `setcmode`, `gettime`, `settime`, `p_cli`, `p_priority`, `conattach`, `condetach`, `getpid`, `getppid`, `getpriority` and `getppriority` check the OS via BDOS fn 12 before calling.
>
> Time stamp format shared by `getftime`, `setftime`, `gettime` and `settime`: `DW` day count (day 1 = 1 January 1978, little endian), `DB` hour (packed BCD), `DB` minute (packed BCD). `getftime` leaves two stamps in the FCB (`+18h`, `+1Ch`), `setftime` takes the same two back-to-back (8 bytes), `gettime`/`settime` use one stamp plus an optional fifth seconds byte (DOS Plus only; elsewhere `gettime` returns the BCD seconds in its result and `settime` resets them to zero).
>
> Note: `getpcode`/`setpcode`/`getcmode`/`setcmode` use a 16-bit result, so `0xFFFF` doubles as the "unsupported" marker (it is never a valid return code).

### Structures & Macros

```c
typedef struct dpb_t {
    unsigned int spt;       /* Sectors per track */
    unsigned char bsh;      /* Block shift factor */
    unsigned char blm;      /* Block mask */
    unsigned char exm;      /* Extent mask */
    unsigned int dsm;       /* Maximum block number (total blocks - 1) */
    unsigned int drm;       /* Total directory entries - 1 */
    unsigned char al0;      /* Directory allocation bitmask 0 */
    unsigned char al1;      /* Directory allocation bitmask 1 */
    unsigned int cks;       /* Directory check vector size */
    unsigned int off;       /* Track offset */
} dpb_t;

#define OSTYPE_MULTIUSER 0x04

#define fcb_is_sys(fcb)     ((fcb)[10] & 0x80)
#define fcb_is_ro(fcb)      ((fcb)[9] & 0x80)
#define fcb_set_sys(fcb)    ((fcb)[10] |= 0x80)
#define fcb_set_ro(fcb)     ((fcb)[9] |= 0x80)
#define fcb_clear_sys(fcb)  ((fcb)[10] &= 0x7F)
#define fcb_clear_ro(fcb)   ((fcb)[9] &= 0x7F)
```

---

## 2. Environment & String Utilities (`util.h`)

Implemented in [`util.c`](util.c:1). Environment variables are loaded from `ENV.DAT` (searched on current drive, user 0, and CCP drive).

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `getenv` | `char *getenv(const char *name)` | Looks up variable `name` in `ENV.DAT`. Returns pointer to value string or `NULL` if not found. |
| `printenv` | `void printenv(FILE *fp)` | Writes all loaded environment variables to stream `fp` in `NAME=VALUE\n` format. |
| `istrcmp` | `int istrcmp(char *p1, char *p2, int p)` | Case-insensitive string comparison. If `p` is non-zero, performs prefix match (returns 0 if `p1` is a prefix of `p2`). |

---

## 3. Console I/O & Terminal Control (`conio.h`)

Implemented in [`conio.c`](conio.c:1). Supports standard CP/M-86 console I/O, VT52 terminal escape sequences, status line management, and screen modes.

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `kbhit` | `int kbhit(void)` | Checks if a console key is waiting via BDOS fn 6 (`DL=0xFE`). Returns non-zero if key available. |
| `getch` | `int getch(void)` | Reads character from console via BDOS fn 6 (`DL=0xFF`) with internal 64-byte lookahead buffer. |
| `cputc` | `int cputc(char c)` | Writes single character `c` to console via BDOS fn 2. |
| `cputs` | `int cputs(char *str)` | Writes NUL-terminated string `str` to console via BDOS fn 2. |
| `gotoxy` | `void gotoxy(int x, int y)` | Positions cursor at `(x, y)` (0-based) using VT52 direct cursor positioning sequence `ESC Y y+32 x+32`. |
| `clrscr` | `void clrscr(void)` | Clears the entire screen (`ESC E`). |
| `clreos` | `void clreos(void)` | Clears from cursor position to end of screen (`ESC J`). |
| `clreol` | `void clreol(void)` | Clears from cursor position to end of line (`ESC K`). |
| `insline` | `void insline(void)` | Inserts a blank line at cursor position (`ESC L`). |
| `delline` | `void delline(void)` | Deletes line at cursor position (`ESC M`). |
| `delchar` | `void delchar(void)` | Deletes character at cursor position (`ESC N`). |
| `cursor` | `void cursor(int cmd)` | Controls cursor state (`CURSOR_OFF`, `CURSOR_ON`, `CURSOR_SAVE`, `CURSOR_RESTORE`). |
| `statline` | `void statline(int on)` | Enables (`STATLINE_ON` = 1) or disables (`STATLINE_OFF` = 0) the 25th status line across CP/M-86 1.1 (`ESC 1`/`ESC 0`) and CCP/M / DOS Plus (`xstatline`). |
| `wrapline` | `void wrapline(int on)` | Enables (`WRAPLINE_ON` = 1, `ESC v`) or disables (`WRAPLINE_OFF` = 0, `ESC w`) line wrapping at column 80. |
| `textcolor` | `void textcolor(int fg)` | Sets text foreground color via `ESC j ESC b <fg> ESC k`. |
| `setstatus` | `void setstatus(char *s)` | Sets status line text in CP/M-86 1.1 sysdata area (offset `0x32`, up to 14 chars). Supports escape tokens `\u` (uppercase), `\l` (lowercase), `\s` (space), `\\` (backslash). |
| `scrmode` | `void scrmode(int m)` | Sets display mode (`SCRMODE_DEFAULT`, `SCRMODE_COL80`, `SCRMODE_COL40`, `SCRMODE_MONO`, `SCRMODE_COLOR`). |
| `crtreset` | `void crtreset(void)` | Resets terminal attributes and modes to default settings. On DOS Plus (non-Concurrent BDOS 41h), first re-enters the active BIOS text mode through the installed INT 10h hook, clearing the screen and homing the cursor to reset XIOS scrolling state. Graphics modes are not re-entered. |

### Constants

```c
#define WRAPLINE_OFF    0
#define WRAPLINE_ON     1

#define STATLINE_OFF    0
#define STATLINE_ON     1

#define CURSOR_OFF      0
#define CURSOR_ON       1
#define CURSOR_SAVE     2
#define CURSOR_RESTORE  3

#define SCRMODE_DEFAULT 0 
#define SCRMODE_COL80   1
#define SCRMODE_COL40   2
#define SCRMODE_MONO    4
#define SCRMODE_COLOR   8
```

---

## 4. Directory & Disk FCB Handling (`dirent.h`)

Implemented in [`dirent.c`](dirent.c:1). Provides high-level directory reading, multi-extent aggregation, sorting, and FAT filesystem detection.

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `dirent_load` | `int dirent_load(char *path, dirent_t **root, int *ouser, int *odrive, int sort_order, int all_extents)` | Parses `path` (e.g. `[user/][drv:]filespec`), searches directory, constructs linked list of `dirent_t` entries at `*root`, aggregates file blocks, and optionally sorts. Returns 0 on success. |
| `dirent_first` | `int dirent_first(char *fcb, dirent_t **root)` | Finds first directory match (BDOS fn 17 / `11h`) and allocates initial `dirent_t` structure. |
| `dirent_next` | `int dirent_next(char *fcb, dirent_t *root, dirent_t **last, int all_extents)` | Finds next match (BDOS fn 18 / `12h`), merges extent blocks into existing matching entry or appends new entry. Returns 0 on success, 255 at end of directory. |
| `dirent_sort` | `void dirent_sort(dirent_t **root, int sort_order)` | Sorts linked list of `dirent_t` entries in-place using merge sort (`sort_order` > 0 for ascending, < 0 for descending). |
| `dirent_free` | `int dirent_free(dirent_t *root)` | Frees all dynamically allocated memory in `dirent_t` linked list. |
| `dirent_is_fat` | `int dirent_is_fat(int drive)` | Probes whether `drive` (0 = `A:`, etc.) is an MS-DOS / FAT formatted disk (returns 1) or native CP/M filesystem (returns 0). |
| `dirent_fcb` | `int dirent_fcb(char *fcb, dirent_t *d)` | Populates FCB buffer `fcb` (12 bytes) from directory entry `d`. Returns entry status byte (user number). |

### Structures & Macros

```c
typedef struct _dirent_t {
    unsigned char entry[12+1];   /* [0]=user, [1..8]=name, [9..11]=ext, [12]=NUL */
    int drive;                   /* Drive index (0 = A, 1 = B, ...) */
    unsigned long blocks;        /* Total allocated blocks across extents */
    unsigned long drive_blocks;  /* Total disk capacity in blocks */
    unsigned long block_size;    /* Block size in bytes (e.g. 1024, 2048, 4096) */
    int is_fat;                  /* 1 if drive is FAT, 0 if CP/M */
    unsigned long fcbs;          /* Number of directory extents aggregated */
    struct _dirent_t *next;      /* Pointer to next directory entry */
} dirent_t;

#define dirent_is_sys(a) ((a)->entry[10] & 0x80)
#define dirent_is_ro(a)  ((a)->entry[9] & 0x80)
```

### Aztec C base-library companions (`fcbinit`, `getusr`, `setusr`, `rstusr`)

These four routines are **not** part of `util.lib`: they come from the Aztec C86 runtime (`fcbinit.asm`, `user.asm` in `aztec42/src/cpm86`) and are linked from `-lc86`. `util.lib` deliberately does not reimplement them; every FCB-based function in this library (`dirent_*`, `getftime`, `setftime`, `rm`, ...) expects an FCB built by `fcbinit` and the user area selected with `setusr`. `fcbinit` is declared in Aztec's `<cpm.h>`; `getusr`/`setusr`/`rstusr` have no header declaration and are used with implicit `int` return (as `dirent.c` and `rm.c` do).

| Function | Signature | Description |
|----------|-----------|-------------|
| `fcbinit` | `int fcbinit(char *name, void *fcb)` | Parses `[user/][d:]name[.ext]` into the CP/M FCB at `fcb`. Returns the **user number** from the prefix (`0`-`255`), `255` if there is no `user/` prefix (meaning "keep the current user"), or `-1` if the drive letter is invalid (not a letter). |
| `getusr` | `int getusr(void)` | BDOS fn 32 (`20h`) with `E=0FFh`: returns the current user number. |
| `setusr` | `int setusr(int user)` | Remembers the current user number in a single static byte, then selects `user` with BDOS fn 32. `setusr(255)` only remembers the current user and does not change it, so it can be called directly with the value returned by `fcbinit`. |
| `rstusr` | `int rstusr(void)` | Restores the user number saved by the last `setusr`. |

**`fcbinit` details**

- Name syntax: optional `NN/` user prefix (decimal digits followed by `/`), optional `d:` drive (upper or lower case), then `NAME[.EXT]`. Name and extension are upper-cased; a `*` is expanded to `?` for the rest of the field; a name longer than 8 characters or an extension longer than 3 is truncated.
- FCB bytes written: `+0` drive (`0` = default, `1` = `A:` ... `16` = `P:`), `+1..+8` name and `+9..+11` extension (blank padded), and `+12..+15` (`EX`, `S1`, `S2`, `RC`) cleared to zero. **Bytes `+16` and above are not touched**: clear the whole buffer first (`memset(fcb, 0, 40)`), as `dirent_load` does, before using the FCB with `getftime`/`setftime` (which use `+18h`..`+1Fh`) or random/sequential I/O.
- Wildcards are not interpreted: an all-blank name stays blank (`dirent_load` replaces it by `????????.???` itself).

**`setusr` / `rstusr` details**

- There is only **one** saved user, so the pair does not nest: a second `setusr` overwrites the value that the first `rstusr` would restore. Always bracket one FCB operation with one `setusr` ... `rstusr`.
- The user number is not stored in the FCB: it must be selected with `setusr` around every BDOS call that uses that FCB (including `getftime`/`setftime`).

**Typical use together with the functions of this library**

```c
char fcb[40];
char stamps[8];
int user;

memset(fcb, 0, sizeof(fcb));
user = fcbinit("2/B:REPORT.TXT", fcb);   /* user = 2, drive B: */
if (user == -1)
    return -1;                           /* bad drive letter */

setusr(user);                            /* 255 = keep current user */
if (bdos(15, fcb) != 0xFF) {             /* F_OPEN */
    if (getftime(fcb) != 0xFF) {         /* stamps now at fcb+18h, fcb+1Ch */
        memcpy(stamps, fcb + 0x18, 4);   /* create/access */
        memcpy(stamps + 4, fcb + 0x1C, 4); /* update */
        /* ... modify stamps ... */
        setftime(fcb, stamps);           /* CP/M-86 v4 / DOS Plus only */
    }
    bdos(16, fcb);                       /* F_CLOSE */
}
rstusr();
```

---

## 5. Graphics (`gfx.h`)

Implemented in [`gfx.c`](gfx.c) and [`gfx.asm`](gfx.asm). BIOS on BDOS 22h or >=60h; XIOS on Concurrent 31h..5Fh and DOS Plus 41h. Native XIOS EGA mode 10h requires the patched ccpm86-kernel capability response and EGA/VGA hardware. See [GRAPHICS.md](GRAPHICS.md).

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `gfx_mode` | `int gfx_mode(int mode)` | Returns 0 when issued, -1 for an unsupported request. BIOS supports modes 3/4/6/0Dh..10h; stock XIOS 3/4/6; patched 3.1 XIOS additionally mode 10h after capability/adapter validation. Hides/restores the status line around graphics. |
| `gfx_palette` | `void gfx_palette(int palette)` | CGA background/intensity and palette selection, using BIOS-style packed BX. Use gfx_palette_entry for native EGA. |
| `gfx_palette_entry` | `int gfx_palette_entry(int index, int color)` | Set EGA palette index 0..15 to raw six-bit EGA color 0..63. Returns 0 when issued, -1 for invalid arguments or a non-EGA mode. Kernel mode uses owner-checked BIOS calls; BIOS backend uses AH=10h/AL=00h. |
| `gfx_pixel` | `void gfx_pixel(int x, int y, int color)` | Plot a pixel, with color bit 7 requesting XOR. XIOS uses packed B800 CGA memory or four A000 EGA planes; out-of-bounds coordinates are ignored. |
| `gfx_text` | `void gfx_text(int col, int row, char *str, int color)` | Character-grid text. XIOS draws ASCII in an opaque black cell: 8x8 in CGA, 8x14 in EGA mode 10h. BIOS backend retains BIOS cursor/teletype drawing. |

### Constants

```c
#define CGA_PALETTE_CMW         0x101  /* Cyan / Magenta / White */
#define CGA_PALETTE_GRY         0x100  /* Green / Red / Yellow */

#define MODE_TEXT               3
#define MODE_CGA_320X200        4
#define MODE_CGA_640X200        6
#define MODE_EGA_320X200        13     /* 0x0D: 320x200, 16 colors */
#define MODE_EGA_640X200        14     /* 0x0E: 640x200, 16 colors */
#define MODE_EGA_640X350_MONO   15     /* 0x0F: 640x350, monochrome */
#define MODE_EGA_640X350        16     /* 0x10: 640x350, 16 colors */
```

---

## 6. Date, Time & Epoch (`util.h`)

Implemented in [`time.c`](time.c:1). Provides system clock read/write, file timestamp read/write, and Unix epoch conversion. On CP/M-86 1.1 the clock comes from the sysdata area; on OSes with version > `22h` it comes from `gettime()` / `settime()` (BDOS fn 105/104). File timestamps use `getftime()` / `setftime()` (BDOS fn 102/116). `time.o` depends on `os.o` and must precede it in `util.lib` (see the `Makefile`).

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `getdatetime` | `int getdatetime(struct sysdt *dt)` | Reads system date and time into `*dt`. On CP/M-86 1.1 (version `22h`) reads from sysdata area (`BDOS 31h` + offset `20h`); on version > `22h` uses `gettime()` (BDOS fn 105). Returns 0 on success, -1 on failure (unsupported OS or unset clock). |
| `setdatetime` | `int setdatetime(struct sysdt *dt)` | Sets system date and time from `*dt`. On CP/M-86 1.1 (version `22h`) writes directly to the sysdata area (same as `tod.a86`); on version > `22h` uses `settime()` (BDOS fn 104). Returns 0 on success, -1 if unsupported. |
| `getfdatetime` | `int getfdatetime(char *fcb, struct sysdt *dt, int which)` | Reads a file timestamp from an open FCB via `getftime()` (BDOS fn 102). `which=0` reads the create/access stamp (FCB+`18h`); `which=1` reads the update stamp (FCB+`1Ch`). Returns 0 on success, -1 on failure or unsupported. |
| `setfdatetime` | `int setfdatetime(char *fcb, struct sysdt *create_dt, struct sysdt *update_dt)` | Sets file timestamps on an open FCB via `setftime()` (BDOS fn 116, CP/M-86 v4 only). Either pointer may be `NULL` to leave that stamp unchanged (the current stamp is read back from the FCB first). Returns 0..3 on success, -1 on failure or unsupported. |
| `epoch` | `long epoch(struct sysdt *dt)` | Converts `*dt` to a Unix epoch timestamp (seconds since 1970-01-01 00:00:00 UTC). Returns `EPOCH_ERR` (`-1L`) if `dt` is `NULL` or the date/time is out of range (year must be 1970..2099). |
| `epoch_now` | `long epoch_now(void)` | Reads the system clock via `getdatetime()` and returns the Unix epoch timestamp. Returns `EPOCH_ERR` (`-1L`) on error. |
| `gettime` | `int gettime(char *stamp)` | BDOS fn 105 (`69h`, `T_GET`): reads the system date/time into `stamp` (5 bytes): `DW` days since 1/1/78 (day 1 = 1 January 1978), `DB` BCD hour, `DB` BCD minute, and on DOS Plus `DB` BCD seconds. Returns BCD seconds, or -1 if unsupported (version <= `22h`). Low-level; prefer `getdatetime()`. |
| `settime` | `int settime(char *stamp)` | BDOS fn 104 (`68h`, `T_SET`): sets the system date/time from a `gettime`-style stamp. Seconds are reset to zero except on DOS Plus. Returns 0, or -1 if unsupported. Low-level; prefer `setdatetime()`. |

### Structures & Constants

```c
struct sysdt {
    int year;   /* 2000..2099 */
    int mon;    /* 1..12 */
    int day;    /* 1..31 */
    int hour;   /* 0..23 */
    int min;    /* 0..59 */
    int sec;    /* 0..59 */
};

#define EPOCH_ERR  (-1L)   /* returned by now() / epoch() on error */
```

---

## 7. Submit Files & Process Chaining (`sub.h`)

Implemented across [`sub.c`](sub.c:1), [`subfile.c`](subfile.c:1), and [`os.asm`](os.asm:1). Enables creating, appending, and expanding `$$$.sub` batch files and chaining program execution under the CP/M-86 CCP.

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `sub_open` | `int sub_open(int flags)` | Opens or creates `$$$.sub`. `SUB_CREATE`: deletes any existing `$$$.sub` and starts fresh. `SUB_APPEND`: opens existing `$$$.sub` to push commands on top (creates if absent). Returns 0 on success, -1 on error. |
| `sub_append` | `int sub_append(char *cmd)` | Pushes one 128-byte record (command line uppercased, max 125 chars) to open `$$$.sub`. Returns 0 on success, -1 on error (or if file reaches 128 record CCP limit). |
| `sub_close` | `int sub_close(void)` | Closes open `$$$.sub`. Returns 0 on success, -1 on error. |
| `sub_records` | `int sub_records(void)` | Returns number of records in existing `$$$.sub` (0 if none). Read-only; leaves file unchanged. |
| `sub_abort` | `int sub_abort(void)` | Undoes changes since `sub_open`: deletes file if created, or restores original record count if appended. Returns 0 on success, -1 on error. |
| `sub_delete` | `int sub_delete(void)` | Deletes `$$$.sub` via BDOS fn 19 (`13h`). |
| `sub_exit` | `void sub_exit(void)` | Sets CCP submit flag (`MDSUBE` at offset `0805h`) and executes warm boot (BDOS fn 0): the CCP immediately executes the top command in `$$$.sub`. Does not return. |
| `p_chain` | `void p_chain(char *cmd, int submode)` | Chains directly to `cmd` via BDOS fn 47 (`2Fh`). If `submode` is non-zero, sets CCP submit flag so `$$$.sub` resumes after `cmd` terminates. |
| `sub_active` | `int sub_active(void)` | Checks whether CCP is currently in submit mode (returns 1 if program was started from `$$$.sub`, 0 if interactive keyboard, -1 if unknown CCP). |
| `sub_cmddrv` | `int sub_cmddrv(void)` | Drive prefix of the running CCP command line (1 = `A:`, ..., 16 = `P:`), or 0 if none/unknown. |
| `sub_load` | `int sub_load(char *cmd)` | Reads and expands `"[d:]file[.typ] [p1 p2 ...]"` following DR SUBMIT substitution rules (`$1`..`$9`, `$$`, `^A`..`^Z`). Returns number of expanded lines (> 0) or `SUBERR_*` code (≤ 0). |
| `sub_line` | `char *sub_line(int i)` | Returns pointer to expanded command line `i` (0 = first line). |
| `sub_lineno` | `int sub_lineno(int i)` | Returns original source line number for loaded line `i`. |
| `sub_name` | `char *sub_name(void)` | Returns actual file name opened by `sub_load` (e.g. `B:BACKUP.SUB`). |
| `sub_errline` | `int sub_errline(void)` | Returns source line number of last parsing error (0 if none). |
| `sub_errmsg` | `char *sub_errmsg(int err)` | Returns human-readable error prefix for a `SUBERR_*` code. |
| `sub_pushall` | `int sub_pushall(void)` | Pushes all lines loaded by `sub_load` onto open `$$$.sub` in reverse order (stack order). Returns 0 on success, -1 on error. |
| `sub_readln` | `int sub_readln(FILE *fp, char *buf, int size)` | Reads one text line (accepts CR LF, LF, or CR; stops at `^Z` or EOF). Returns line length, `SUB_RD_EOF` (-1), or `SUB_RD_LONG` (-2). |

### Constants

```c
#define SUB_CREATE      0
#define SUB_APPEND      1

#define SUB_MAX_LINES   64
#define SUB_LINE_LEN    126
#define SUB_PARAM_LEN   32

#define SUBERR_EMPTY    0
#define SUBERR_OPEN    -1
#define SUBERR_LONG    -2
#define SUBERR_LINES   -3
#define SUBERR_CTRL    -4
#define SUBERR_READ    -5

#define SUB_RD_EOF     -1
#define SUB_RD_LONG    -2
```

---

## 8. Debug Utilities (`debug.h`)

Implemented in [`debug.c`](debug.c:1).

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `debug_dump_hex` | `unsigned long debug_dump_hex(FILE *fd, char *data, unsigned int size, unsigned long offset, int raw)` | Formats and prints hexadecimal and ASCII dump of memory buffer `data` of length `size` to file stream `fd`. Displays 6-digit hex offsets, 16 hex bytes per line, and ASCII representation. If `raw` is non-zero, outputs only hex bytes without offset and ASCII sidebar. Returns next updated offset. |
