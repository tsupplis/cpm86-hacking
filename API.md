# util.lib API Reference

`util.lib` is the standard runtime utility library for CP/M-86 tools and programs in this repository. It provides OS detection, low-level BDOS/BIOS/hardware access, console manipulation, directory traversal, CGA graphics, timekeeping, environment variable handling, and submit (`$$$.sub`) batch chaining.

---

## Table of Contents

1. [Operating System & Low-Level Services (`util.h`)](#1-operating-system--low-level-services-utilh)
2. [Environment & String Utilities (`util.h`)](#2-environment--string-utilities-utilh)
3. [Console I/O & Terminal Control (`conio.h`)](#3-console-io--terminal-control-conioh)
4. [Directory & Disk FCB Handling (`dirent.h`)](#4-directory--disk-fcb-handling-direnth)
5. [CGA Graphics (`gfx.h`)](#5-cga-graphics-gfxh)
6. [Date, Time & Epoch (`epoch.h`)](#6-date-time--epoch-epochh)
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
| `crtreset` | `void crtreset(void)` | Resets terminal attributes and modes to default settings. |

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

---

## 5. CGA Graphics (`gfx.h`)

Implemented in [`gfx.asm`](gfx.asm:1). Direct BIOS `INT 10h` primitives for CGA graphics modes.

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `gfx_mode` | `void gfx_mode(int mode)` | Sets video mode via BIOS `INT 10h` (`AH=00h`). E.g. `MODE_CGA_320X200` (4), `MODE_CGA_640X200` (6), `MODE_TEXT` (3). |
| `gfx_palette` | `void gfx_palette(int palette)` | Sets CGA color palette via BIOS `INT 10h` (`AH=0Bh`). E.g. `CGA_PALETTE_GRY` (`0x100`), `CGA_PALETTE_CMW` (`0x101`). |
| `gfx_pixel` | `void gfx_pixel(int x, int y, int palette)` | Plots pixel at coordinates `(x, y)` with color/palette index `palette` via BIOS `INT 10h` (`AH=0Ch`). |

### Constants

```c
#define CGA_PALETTE_CMW   0x101  /* Cyan / Magenta / White */
#define CGA_PALETTE_GRY   0x100  /* Green / Red / Yellow */

#define MODE_CGA_320X200  4
#define MODE_CGA_640X200  6
#define MODE_TEXT         3
```

---

## 6. Date, Time & Epoch (`epoch.h`)

Implemented in [`epoch.c`](epoch.c:1). Reads system clock from CP/M-86 1.1 sysdata area and converts to/from Unix epoch timestamps (seconds since 1970-01-01 00:00:00 UTC).

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `sys_datetime` | `int sys_datetime(struct sysdt *dt)` | Reads system date and time from CP/M-86 1.1 sysdata area (`BDOS 31h` + offset `20h`). Populates `struct sysdt`. Returns 0 on success, -1 on failure. |
| `epoch_make` | `long epoch_make(int y, int m, int d, int hh, int mm, int ss)` | Computes Unix epoch timestamp (seconds since 1970-01-01 UTC) for given calendar date/time (`y` = 1970..2099). Returns `EPOCH_ERR` (`-1L`) on invalid date/time. |
| `epoch_now` | `long epoch_now(void)` | Reads current system time via `sys_datetime` and converts to Unix epoch timestamp. Returns `EPOCH_ERR` (`-1L`) on error. |

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

#define EPOCH_ERR  (-1L)
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
