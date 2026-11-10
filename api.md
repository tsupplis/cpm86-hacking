# util.lib API Reference

`util.lib` provides CP/M-86 system services, console I/O, directories, graphics, clocks, environment variables, and command chaining.

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

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `osver` | `int osver(void)` | Returns the operating system version. |
| `ostype` | `int ostype(void)` | Returns the operating system type flags. |
| `getcurdrv` | `int getcurdrv(void)` | Returns the default drive (0 = `A:`, 1 = `B:`, ...). |
| `setcurdrv` | `int setcurdrv(int drv)` | Selects the default drive (0 = `A:`, 1 = `B:`, ...). |
| `getccpdrv` | `int getccpdrv(void)` | Returns the command's load drive (0..15), or `0x0FFF` if unknown. |
| `bdosx` | `int bdosx(int cx, int dx, int *es, int *bx)` | Calls a BDOS service and retrieves its results. |
| `delay` | `void delay(unsigned int ticks)` | Waits for the requested number of timer ticks. |
| `bioskey` | `int bioskey(void)` | Reads a key with its character and scan code. |
| `pckey` | `int pckey(void)` | Reads a raw PC keyboard scan code. |
| `dpb_load` | `void dpb_load(int drive, dpb_t *dpb)` | Reads disk geometry for `drive` into `*dpb` (0 = `A:`, etc.). |
| `getftime` | `int getftime(char *fcb)` | Reads an open file's dates, times, and password mode. |
| `setftime` | `int setftime(char *fcb, char *stamps)` | Sets an open file's creation/access and modification times. |
| `getpcode` | `unsigned int getpcode(void)` | Returns the program return code. |
| `setpcode` | `unsigned int setpcode(unsigned int code)` | Sets the program return code. |
| `getcmode` | `unsigned int getcmode(void)` | Returns the console-mode flags. |
| `setcmode` | `unsigned int setcmode(unsigned int mode)` | Sets the console-mode flags. |
| `p_cli` | `int p_cli(char *cmd)` | Launches a native command and transfers console control. |
| `p_spawn` | `int p_spawn(char *cmd)` | Launches a native or DOS command, then reacquires the console. |
| `ms_p_cli` | `int ms_p_cli(char *opened_fcb, char *cmd)` | Launches a DOS program identified by an open file. |
| `p_priority` | `int p_priority(int priority)` | Sets the process's scheduling priority. |
| `conattach` | `int conattach(void)` | Waits for and acquires the default console. |
| `condetach` | `int condetach(void)` | Releases the console for background execution. |
| `getpid` | `unsigned int getpid(void)` | Returns this process's ID, or `0xFFFF` if unsupported. |
| `getppid` | `unsigned int getppid(void)` | Returns the parent's ID, 0 if none, or `0xFFFF` if unsupported. |
| `getpriority` | `int getpriority(void)` | Returns this process's priority (0..255), or -1 if unsupported. |
| `getppriority` | `int getppriority(void)` | Returns the parent's priority (0..255), or -1 if absent/unsupported. |

### Usage notes

- `osver`: e.g. `0x22` = CP/M-86 1.1, `0x41` = DOS Plus.
- `ostype`: a zero system type is reported as `0x10`; test `OSTYPE_MULTIUSER` for Concurrent systems.
- `bdosx`: `cx` selects the service, `dx` supplies its parameter. Returns AX's low byte; writes ES/BX through the pointers.
- `bioskey`: character in the low byte, scan code in the high byte; -1 on MP/M/Concurrent.
- `getftime`/`setftime`: return 0..3 on success, `0xFF` on failure/unsupported. Require versions >= `0x30`/`0x41`, respectively. See [date formats](#date-formats).
- `getpcode`/`setpcode`/`getcmode`/`setcmode`: require CP/M 3 or later, excluding MP/M II; return `0xFFFF` if unsupported. Setters return the resulting value; passing `0xFFFF` reads without changing it.
- `p_priority`/`conattach`: MP/M/Concurrent only. `condetach` also supports versions >= `0x41`. Return 0, or -1 if unsupported.
- Priorities range from 0 (highest) to 255 (lowest); default 200.
- Process IDs require MP/M/Concurrent; treat them as unsigned values.

Program return codes: `0x0000`..`0xFEFF` are nonfatal; `0xFF00`..`0xFF7F` stop
subsequent `:` commands; `0xFFFD` means hardware error, `0xFFFE` means Ctrl-C.
Console-mode flags are OS-specific.

After `condetach`, console I/O reattaches on MP/M/Concurrent. On CP/M-86 v4,
keyboard use terminates the detached program; only native BDOS calls are allowed.

For FCB construction and user selection, see [base-library companions](#aztec-c-base-library-companions-fcbinit-getusr-setusr-rstusr).
For chaining, see [section 7](#7-submit-files--process-chaining-subh).

### Concurrent program launch

`p_cli` requires MP/M/Concurrent and accepts up to 127 characters.
Returns 0, or `0xFF` on failure/unsupported.

`p_spawn` accepts a nonempty command of at most 127 characters and an
executable name of at most 15 characters; BAT commands are limited to 121.
With PCTMP on Concurrent versions `0x31`..`0x41`, it supports CMD, COM, EXE,
and BAT files, using PCTMP ORDER for extensionless names. Unqualified names
may also find system files on the system disk on versions `0x31` and `0x41`.
Otherwise, native command resolution applies.
Returns loader status (0 = success), or -1 for invalid input/resolution
failure; this is not the child's exit code.

### Low-level DOS program launch

`ms_p_cli` requires PCTMP on Concurrent versions `0x31`..`0x41`, an open
36-byte COM/EXE FCB, and a full NUL-terminated command of at most 127 characters.
Uses the current user; consumes the FCB, which must not be closed again.
No filename search, built-ins, CMD, or BAT support. Not reentrant; return
after DOS termination is not guaranteed. Prefer `p_spawn` for ordinary launches.
Returns loader status (0 = success), or -1 if unsupported/overlong.

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

Environment variables come from `ENV.DAT`, searched on the current drive/user,
then user 0, then the command drive's user 0. Values are cached.

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `getenv` | `char *getenv(const char *name)` | Looks up variable `name` in `ENV.DAT`. Returns pointer to value string or `NULL` if not found. |
| `printenv` | `void printenv(FILE *fp)` | Writes all loaded environment variables to stream `fp` in `NAME=VALUE\n` format. |
| `istrcmp` | `int istrcmp(char *p1, char *p2, int p)` | Case-insensitive string comparison. If `p` is non-zero, performs prefix match (returns 0 if `p1` is a prefix of `p2`). |

---

## 3. Console I/O & Terminal Control (`conio.h`)

Raw console I/O and display control for the PC's VT52-style console.

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `kbhit` | `int kbhit(void)` | Checks whether console input is waiting. |
| `getch` | `int getch(void)` | Reads a console character without echo. |
| `cputc` | `int cputc(char c)` | Writes one console character; returns OS output status. |
| `cputs` | `int cputs(char *str)` | Writes a NUL-terminated console string. Returns 0. |
| `gotoxy` | `void gotoxy(int x, int y)` | Positions the cursor at zero-based row `x`, column `y`. Coordinates must fit the screen. |
| `clrscr` | `void clrscr(void)` | Clears the entire screen. |
| `clreos` | `void clreos(void)` | Clears from cursor position to end of screen. |
| `clreol` | `void clreol(void)` | Clears from cursor position to end of line. |
| `insline` | `void insline(void)` | Inserts a blank line at cursor position. |
| `delline` | `void delline(void)` | Deletes line at cursor position. |
| `delchar` | `void delchar(void)` | Deletes character at cursor position. |
| `cursor` | `void cursor(int cmd)` | Controls cursor state (`CURSOR_OFF`, `CURSOR_ON`, `CURSOR_SAVE`, `CURSOR_RESTORE`). |
| `statline` | `void statline(int on)` | Shows or hides the status line. |
| `wrapline` | `void wrapline(int on)` | Enables (`WRAPLINE_ON` = 1) or disables (`WRAPLINE_OFF` = 0) terminal line wrapping. |
| `textcolor` | `void textcolor(int fg)` | Sets the foreground color without moving the cursor. |
| `setstatus` | `void setstatus(char *s)` | Sets the status-line text. |
| `scrmode` | `void scrmode(int m)` | Selects a display mode. |
| `crtreset` | `void crtreset(void)` | Restores default console settings. |

### Usage notes

- `kbhit` returns nonzero for waiting input, but may return zero while `getch` has buffered input.
- `getch` waits for input, preserves high-bit bytes, and does no editing or key decoding.
- `statline` sends the console escapes `ESC 1` (on) / `ESC 0` (off) on every version; there is no XIOS call. `gfx_mode` turns the status line off for graphics modes and back on when returning to text.
- `textcolor` ignores values below 1; positive values use their low byte.
- `setstatus` supports CP/M-86 1.1: 14 cells, space-padded, lowercase by default. Tokens: `\u` preserves case, `\l` resumes lowercase, `\s` and `\\` insert spaces. Tokens are case-insensitive; unknown tokens are discarded.
- `scrmode` supports CP/M-86 1.1 only. Choose one `SCRMODE_*` value; default resets/clears the console.
- `crtreset` also clears/homes DOS Plus text screens; leaves graphics modes unchanged.

`getch()` leaves programmable function-key expansion unchanged. Applications
selecting raw-key mode must decode keys and restore expanded-key mode before
releasing the console.

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

Lists files, aggregates extents, sorts entries, and identifies FAT disks.

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `dirent_load` | `int dirent_load(char *path, dirent_t **root, int *ouser, int *odrive, int sort_order, int dirent_options)` | Lists files matching a path or pattern. |
| `dirent_first` | `int dirent_first(char *fcb, dirent_t **root)` | Starts a directory search and retrieves the first match. |
| `dirent_next` | `int dirent_next(char *fcb, dirent_t *root, dirent_t **last, int dirent_options)` | Retrieves the next match and updates the entry list. |
| `dirent_sort` | `void dirent_sort(dirent_t **root, int sort_order)` | Sorts entries by filename: positive = ascending, negative = descending. |
| `dirent_free` | `int dirent_free(dirent_t *root)` | Releases an entry list. Returns 0. |
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
    int is_dir;                  /* FAT directory marker */
    int is_hidden;               /* FAT hidden marker; always 0 on CP/M */
    int include_dirs;            /* Retained search option */
    unsigned long fcbs;          /* Number of directory extents aggregated */
    struct _dirent_t *next;      /* Pointer to next directory entry */
} dirent_t;

#define dirent_is_sys(a) ((a)->entry[10] & 0x80)
#define dirent_is_ro(a)  ((a)->entry[9] & 0x80)
#define dirent_is_dir(a) ((a)->is_dir)
#define dirent_is_hidden(a) ((a)->is_hidden)
#define dirent_is_system_group(a) (dirent_is_sys(a) || dirent_is_hidden(a))

#define DIRENT_ALL_EXTENTS 1
#define DIRENT_INCLUDE_DIRS 2
```

### Usage notes

- Search results: 0 = success, 255 = no match/end, -2 = allocation failure.
- `dirent_load` accepts `[user/][drv:]filespec`; returns the list at `*root`. Optional outputs receive user/zero-based drive. Release results with `dirent_free`.
- `dirent_next` uses settings established by `dirent_first` and updates the list tail.
- For `dirent_load`, zero `sort_order` preserves search order.

Combine `DIRENT_ALL_EXTENTS` (all file extents) and `DIRENT_INCLUDE_DIRS`
(include directories; OS version >= `0x41`). Directory and hidden flags
apply only to FAT entries.

### Aztec C base-library companions (`fcbinit`, `getusr`, `setusr`, `rstusr`)

Provided by Aztec C's `-lc86`, not `util.lib`. Declare `fcbinit` through
`<cpm.h>`; the user-selection routines have no supplied header declaration.

| Function | Signature | Description |
|----------|-----------|-------------|
| `fcbinit` | `int fcbinit(char *name, void *fcb)` | Prepares a filename for file operations and returns its user number. |
| `getusr` | `int getusr(void)` | Returns the current user number. |
| `setusr` | `int setusr(int user)` | Saves the current user, then selects `user`. With 255, saves without changing it. |
| `rstusr` | `int rstusr(void)` | Restores the user number saved by the last `setusr`. |

**`fcbinit` details**

- Returns the specified user (0..255), 255 if omitted, or -1 for an invalid drive letter.
- Name syntax: optional `NN/` user prefix (decimal digits followed by `/`), optional `d:` drive (upper or lower case), then `NAME[.EXT]`. Name and extension are upper-cased; a `*` is expanded to `?` for the rest of the field; a name longer than 8 characters or an extension longer than 3 is truncated.
- Only FCB bytes 0..15 are initialized. Clear the full FCB before calling.
- Wildcards are encoded, not searched; an empty filename stays blank.

**`setusr` / `rstusr` details**

- Calls do not nest: each `setusr` replaces the saved user.
- Select the file's user before FCB operations; restore it afterwards.

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

Graphics modes depend on OS and adapter support; see [graphics.md](graphics.md).

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `gfx_mode` | `int gfx_mode(int mode)` | Selects a display mode. |
| `gfx_palette` | `void gfx_palette(int palette)` | Selects the CGA background or palette. |
| `gfx_palette_entry` | `int gfx_palette_entry(int index, int color)` | Changes an EGA palette color. |
| `gfx_dac_entry` | `int gfx_dac_entry(int index, int red, int green, int blue)` | Changes a VGA palette color. |
| `gfx_pixel` | `void gfx_pixel(int x, int y, int color)` | Draws a pixel. |
| `gfx_text` | `void gfx_text(int col, int row, char *str, int color)` | Draws text at a character-grid position. |

### Usage notes

- `gfx_mode`: returns 0 when requested, -1 if unsupported; hides/restores the status line around graphics.
- `gfx_palette`: high byte selects background/intensity or palette operation, low byte its value.
- `gfx_palette_entry`: index 0..15, color 0..63, modes `0x0D`..`0x12`.
- `gfx_dac_entry`: index 0..255, RGB components 0..63, modes `0x11`..`0x13`.
- Both palette-entry functions return 0, or -1 for invalid arguments/mode.
- `gfx_pixel`: ignores out-of-bounds coordinates; color bit 7 requests XOR except in mode `0x13`. In `MODE_HERCULES` bit 0 is the pixel.
- `MODE_HERCULES`: 720x348, 90x43 text cells of 8x8. BIOS backend (BDOS 22h or >=60h): card driven directly after detection. Patched Concurrent XIOS: capability bit 4. Otherwise, or without a Hercules adapter, `gfx_mode` returns -1. Palette functions return -1.
- `gfx_text`: Concurrent/DOS Plus cells have black backgrounds; 8x8 normally, 8x14 in `0x10`, 8x16 in `0x11`/`0x12`.

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
#define MODE_VGA_640X480_MONO   17     /* 0x11: 640x480, 2 colors */
#define MODE_VGA_640X480        18     /* 0x12: 640x480, 16 colors */
#define MODE_VGA_320X200        19     /* 0x13: 320x200, 256 colors */
#define MODE_HERCULES           127    /* 0x7F: 720x348, monochrome */
```

---

## 6. Date, Time & Epoch (`util.h`)

Reads/writes clocks and file timestamps; converts dates to Unix seconds.
Times are treated as UTC.

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `getdatetime` | `int getdatetime(struct sysdt *dt)` | Gets the current date and time. |
| `setdatetime` | `int setdatetime(struct sysdt *dt)` | Sets the current date and time. |
| `getfdatetime` | `int getfdatetime(char *fcb, struct sysdt *dt, int which)` | Gets a file's creation/access or modification time. |
| `setfdatetime` | `int setfdatetime(char *fcb, struct sysdt *create_dt, struct sysdt *update_dt)` | Sets a file's creation/access and modification times. |
| `epoch` | `long epoch(struct sysdt *dt)` | Converts a date and time to Unix seconds. |
| `epoch_now` | `long epoch_now(void)` | Gets the current time in Unix seconds. |
| `gettime` | `int gettime(char *stamp)` | Gets the current date and time in native format. |
| `settime` | `int settime(char *stamp)` | Sets the current date and time from native format. |

### Usage notes

- `getdatetime`/`setdatetime`: return 0, or -1 on failure/unsupported. Supply valid dates; reads do not fully validate them or detect an unset CP/M-86 1.1 clock.
- File operations require an open FCB. `getfdatetime`: `which=0` selects creation/access, nonzero selects modification; returns 0 or -1 on failure/unsupported.
- `setfdatetime`: NULL preserves a time. Requires version >= `0x41`; returns 0..3 on success, -1 for read failure, or `0xFF` for write failure/unsupported.
- `epoch`/`epoch_now`: return `EPOCH_ERR` on failure. `epoch` rejects NULL, invalid dates, years outside 1970..2099, and overflow. With 32-bit `long`, the last supported instant is 2038-01-19 03:14:07 UTC.
- `gettime`/`settime`: require version > `0x22`. `gettime` returns BCD seconds or -1; `settime` returns 0 or -1. Setting seconds is supported only on DOS Plus; elsewhere they become zero.

### Date formats

Packed stamps contain a little-endian 16-bit day count (1 = 1978-01-01),
then BCD hour and minute. Clock buffers reserve a fifth byte for DOS Plus
BCD seconds. File-stamp buffers contain two four-byte stamps: create/access,
then update. Prefer the `sysdt` functions unless packed data is needed.

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

#define EPOCH_ERR  (-1L)   /* returned by epoch_now() / epoch() on error */
```

---

## 7. Submit Files & Process Chaining (`sub.h`)

Builds `$$$.sub` command stacks, expands submit files, and chains programs.

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `sub_open` | `int sub_open(int flags)` | Opens a new or existing submit command stack. |
| `sub_append` | `int sub_append(char *cmd)` | Adds a command to the stack. |
| `sub_close` | `int sub_close(void)` | Closes open `$$$.sub`. Returns 0 on success, -1 on error. |
| `sub_records` | `int sub_records(void)` | Returns number of records in existing `$$$.sub` (0 if none). Read-only; leaves file unchanged. |
| `sub_abort` | `int sub_abort(void)` | Undoes changes made since opening the stack. |
| `sub_restore` | `int sub_restore(int records)` | Restores a previous stack length. |
| `sub_delete` | `int sub_delete(void)` | Attempts to delete `$$$.sub`. Always returns 0, including on failure. |
| `sub_exit` | `void sub_exit(void)` | Exits to the CCP to run the top submit command. Does not return. |
| `p_chain` | `void p_chain(char *cmd, int submode)` | Replaces the program with `cmd`; nonzero `submode` requests submit processing after it finishes. |
| `sub_active` | `int sub_active(void)` | Checks whether CCP is currently in submit mode (returns 1 if program was started from `$$$.sub`, 0 if interactive keyboard, -1 if unknown CCP). |
| `sub_cmddrv` | `int sub_cmddrv(void)` | Drive prefix of the running CCP command line (1 = `A:`, ..., 16 = `P:`), or 0 if none/unknown. |
| `sub_load` | `int sub_load(char *cmd)` | Loads a submit file and substitutes its parameters. |
| `sub_line` | `char *sub_line(int i)` | Returns expanded line `i` (zero-based), or an empty string for an invalid index. |
| `sub_lineno` | `int sub_lineno(int i)` | Returns the line's source number, or 0 for an invalid index. |
| `sub_name` | `char *sub_name(void)` | Returns actual file name opened by `sub_load` (e.g. `B:BACKUP.SUB`). |
| `sub_errline` | `int sub_errline(void)` | Returns source line number of last parsing error (0 if none). |
| `sub_errmsg` | `char *sub_errmsg(int err)` | Returns human-readable error prefix for a `SUBERR_*` code. |
| `sub_pushall` | `int sub_pushall(void)` | Adds loaded lines to the open stack for execution in source order. Returns 0, or -1 on error. |
| `sub_readln` | `int sub_readln(FILE *fp, char *buf, int size)` | Reads one text line. |

### Usage notes

- `sub_open`: `SUB_CREATE` replaces any existing stack; `SUB_APPEND` preserves it, creating one if absent. Returns 0 or -1 on failure.
- `sub_append`: uppercases commands and truncates to 125 characters; maximum 128 commands. Returns 0 or -1 on write failure/full stack.
- `sub_abort`: deletes a newly created stack or restores its original length. Returns 0 or -1 on failure.
- `sub_restore`: current drive/user, 1..128 existing records; 0 deletes the stack. Preserves rollback state. Returns 0 or -1 on failure/invalid count.
- `sub_load`: accepts `"[d:]file[.typ] [p1 p2 ...]"`; substitutions are `$1`..`$9`, `$$`, and `^A`..`^Z`. Returns a positive line count or `SUBERR_*`.
- `sub_readln`: `size` includes NUL. Accepts CR/LF/CRLF; stops at Ctrl-Z/EOF. Returns length, `SUB_RD_EOF`, or `SUB_RD_LONG` (retains prefix, skips remainder).

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

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `debug_dump_hex` | `unsigned long debug_dump_hex(FILE *fd, char *data, unsigned int size, unsigned long offset, int raw)` | Writes a hexadecimal and ASCII dump. |

Output goes to `fd`, 16 bytes per row. Nonzero `raw` omits offsets/ASCII.
Returns the next offset.
