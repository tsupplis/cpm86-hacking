
# util.lib API

Reusable CP/M-86 library for writing `$$$.sub` submit files.

| Function | Signature | Description |
|----------|-----------|-------------|
| `sub_open` | `int sub_open(int flags)` | `SUB_CREATE`: delete any `$$$.sub` and create it. `SUB_APPEND`: open it and push on top of the records already there (creates it if absent). Returns 0 / -1. |
| `sub_append` | `int sub_append(char *cmd)` | Push one 128-byte record (command uppercased, max 125 chars). Fails once the file holds 128 records. Returns 0 / -1. |
| `sub_close` | `int sub_close(void)` | Close. Returns 0 / -1. |
| `sub_records` | `int sub_records(void)` | Records in the existing `$$$.sub` (0 if none). Opens it read-only, writes nothing. |
| `sub_abort` | `int sub_abort(void)` | Undo everything since `sub_open`: delete the file if it was created, else restore its record count. Returns 0 / -1. |
| `sub_delete` | `int sub_delete(void)` | Delete `$$$.sub` if present. |
| `sub_exit` | `void sub_exit(void)` | Set the CCP submit flag and warm boot (BDOS 0): the CCP runs the top record. Does not return. |
| `p_chain` | `void p_chain(char *cmd, int submode)` | Chain to `cmd` (BDOS 47). `submode` non-zero also sets the CCP submit flag so `$$$.sub` runs afterwards. |
| `sub_active` | `int sub_active(void)` | 1 if the CCP is in submit mode (program started from `$$$.sub`), 0 if not, -1 if the CCP is not recognised. |
| `sub_cmddrv` | `int sub_cmddrv(void)` | Drive prefix of the command the CCP is running (1 = A … 16 = P), 0 if none or unknown. |
| `sub_load` | `int sub_load(char *cmd)` | Read and expand `"[d:]file[.typ] [p1 …]"` (`subfile.c`). Returns the line count (> 0) or a `SUBERR_*` code (≤ 0). |
| `sub_line` | `char *sub_line(int i)` | Expanded line `i` (0 = first line of the file). |
| `sub_pushall` | `int sub_pushall(void)` | Push all loaded lines on the open `$$$.sub`, last first. Returns 0 / -1. |
| `sub_name` | `char *sub_name(void)` | File actually opened, e.g. `B:BACKUP.SUB`. |
| `sub_errmsg` | `char *sub_errmsg(int err)` | Message for a `SUBERR_*` code; the file name follows it. |
| `sub_errline` | `int sub_errline(void)` | Source line of the last error, 0 if none. |
| `sub_lineno` | `int sub_lineno(int i)` | Source line number of loaded line `i`. |
| `sub_readln` | `int sub_readln(FILE *fp, char *buf, int size)` | Read one text line (CR LF, LF or CR end, stops at `^Z`). Returns its length, `SUB_RD_EOF`, or `SUB_RD_LONG` if it did not fit. Also used for `.dat` files. |

Typical use (this is all of `MSUB`):

```c
n = sub_load("JOB P1 P2");                  /* read + expand, nothing written */
if (n <= 0) { /* sub_errmsg(n), sub_name(), sub_errline() */ }
sub_open(sub_active() == 1 ? SUB_APPEND : SUB_CREATE);
sub_pushall();
sub_close();                                /* on failure: sub_abort()       */
sub_exit();                                 /* CCP runs it, no return        */
```

`sub_exit`, `p_chain`, `sub_active` and `sub_cmddrv` live in `os.asm` and only
touch the CCP after recognising the CP/M-86 1.1 CCP (version 22h and the
`MENUSIG`/`SUBNAM` signature).

Records are written last-command-first (stack order) so the CCP executes them top-down.
See [submit-flow.md](submit-flow.md) for the full hand-over between `menu.cmd`,
the BDOS and the CCP.

