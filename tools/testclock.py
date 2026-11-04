#!/usr/bin/env python3
"""Execute source-derived clock/MEM routines and the built WAIT command in emu2."""
from datetime import date
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent.parent


def asm(text):
    text = re.sub(r"\b(?:byte|word) ptr ([\w+]+)", r"[\1]", text)
    text = re.sub(r"\blea\s+(\w+),\s*(\w+)", r"mov \1,\2", text)
    return text


def execute(folder, code):
    (folder / "test.asm").write_text("bits 16\ncpu 8086\norg 100h\n" + code)
    subprocess.run(["nasm", "-f", "bin", "test.asm", "-o", "test.com"],
                   cwd=folder, check=True)
    result = subprocess.run(["emu2", "test.com"], cwd=folder,
                            capture_output=True, timeout=15)
    assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)


EXIT = """
ok: mov ax,4c00h
 int 21h
failed: mov ax,4c01h
 int 21h
"""


def calendar(folder):
    cases = []
    for year in (1978, 1979, 1980, 1981, 1999, 2000, 2001, 2024, 2026,
                 2028, 2099, 2100, 2101, 2156):
        for month, day in ((1, 1), (2, 28), (3, 1), (12, 31)):
            value = date(year, month, day)
            days = (value - date(1978, 1, 1)).days + 1
            if days <= 65535:
                cases.append((year, month, day, days))
        if year % 4 == 0 and (year % 100 != 0 or year % 400 == 0):
            cases.append((year, 2, 29, (date(year, 2, 29) - date(1978, 1, 1)).days + 1))
    for name in ("clock.a86", "pce/pceclock.a86"):
        source = (ROOT / name).read_text()
        routines = asm(source[source.index("leap_days:"):source.index("cpm_set_time:")])
        body = "mov dx,bdos\nmov ax,25e0h\nint 21h\n"
        for year, month, day, days in cases:
            body += f"""
 mov word [tm_buffer],{year}
 mov byte [tm_buffer+2],{day - 1}
 mov byte [tm_buffer+3],{month - 1}
 mov byte [tm_buffer+5],23
 mov byte [tm_buffer+4],59
 mov byte [tm_buffer+7],58
 mov word [expected],{days}
 call cpm31_set_time
"""
        body += "jmp ok\n" + EXIT + """
bdos:
 cmp cx,68h
 jne failed
 mov ax,[expected]
 cmp [stamp_year],ax
 jne failed
 cmp byte [stamp_hours],23h
 jne failed
 cmp byte [stamp_mins],59h
 jne failed
 cmp byte [stamp_secs],58h
 jne failed
 iret
tm_buffer times 8 db 0
stamp_year dw 0
stamp_hours db 0
stamp_mins db 0
stamp_secs db 0
expected dw 0
month_day dw 0,31,59,90,120,151,181,212,243,273,304,334
""" + routines
        execute(folder, body)
    print(f"PASS: {len(cases) * 2} actual T_SET stamps, leap boundaries and century rules")


def rtc(folder):
    source = (ROOT / "clock.a86").read_text()
    prefix = source[source.index("time_read:"):source.index("print_time:")]
    # Select the ATCLOCK branch of the source's conditional assembly.
    prefix = re.sub(r"ELSE.*?ENDIF", "", prefix, flags=re.S)
    prefix = re.sub(r"(?m)^(?:IF ATCLOCK|ENDIF)\s*$", "", prefix)
    decode = source[source.index("decode_at_time:"):source.index("\nELSE", source.index("decode_at_time:"))]
    bcd = source[source.index("bcd_conv_byte:"):source.index("leap_days:")]
    for failure in (0, 2, 4):
        execute(folder, """
 mov dx,bdos
 mov ax,25e0h
 int 21h
 mov dx,bios
 mov ax,251ah
 int 21h
 jmp time_read
""" + EXIT + f"""
bios:
 push bp
 mov bp,sp
 and word [bp+6],0fffeh
 cmp ah,{failure}
 jne bios_good
 or word [bp+6],1
 pop bp
 iret
bios_good:
 cmp ah,2
 jne bios_date
 mov cx,2359h
 mov dx,5800h
 pop bp
 iret
bios_date:
 mov cx,2026h
 mov dx,1104h
 pop bp
 iret
bdos:
 mov ax,1431h
 iret
print_str:
 cmp si,msg_interr
 jne failed
 cmp byte [printed],0
 jne failed
 mov byte [error],1
 ret
print_time:
 mov byte [printed],1
 ret
cpm_set_time:
 jmp failed
cpm31_set_time:
 cmp word [tm_buffer],2026
 jne failed
 cmp byte [tm_buffer+5],23
 jne failed
 ret
quit:
 cmp byte [error],{int(failure != 0)}
 jne failed
 cmp byte [printed],{int(failure == 0)}
 jne failed
 jmp ok
msg_at_unsupported db 0
msg_86err db 0
msg_interr db 0
error db 0
printed db 0
tm_buffer times 8 db 0
""" + asm(prefix + "\n" + decode + "\n" + bcd))
    # Test the decoder's register-preservation contract directly.
    execute(folder, """
 mov ax,1234h
 mov bx,5678h
 mov cx,2359h
 mov dx,5800h
 call decode_at_time
 cmp ax,1234h
 jne failed
 cmp bx,5678h
 jne failed
 jmp ok
""" + EXIT + "tm_buffer times 8 db 0\n" + asm(decode + "\n" + bcd))
    print("PASS: RTC success, both BIOS carry failures and decoder register preservation")


def wait(folder):
    for version in (0x22, 0x1430, 0x1431, 0x1432, 0x1441, 0x1041):
        for seconds in (1, 9):
            execute(folder, f"""
 mov dx,bdos
 mov ax,25e0h
 int 21h
 mov dx,bios
 mov ax,251ah
 int 21h
 mov ax,cs
 add ax,(payload-$$+100h)/16
 mov [entry+2],ax
 mov ds,ax
 mov es,ax
 mov byte [80h],1
 mov byte [81h],{ord('0') + seconds}
 jmp far [cs:entry]
""" + EXIT + f"""
bdos:
 cmp cx,0ch
 je version
 cmp cx,8dh
 je delay
 or cx,cx
 jne failed
 cmp byte [cs:delays],{int(version != 0x22)}
 jne failed
 cmp word [cs:version_number],22h
 jne wait_done
 mov ax,[cs:ticks]
 sub ax,0fff7h
 cmp ax,{seconds * 18}
 jb failed
 cmp ax,{seconds * 18 + 7}
 jae failed
wait_done:
 jmp ok
version:
 mov ax,{version}
 iret
delay:
 cmp dx,{seconds * 60}
 jne failed
 inc byte [cs:delays]
 iret
bios:
 cmp word [cs:version_number],22h
 jne failed
 add word [cs:ticks],7
 mov dx,[cs:ticks]
 iret
entry dw 100h,0
version_number dw {version}
delays db 0
ticks dw 0fff0h
align 16
payload: incbin "{ROOT / 'wait.cmd'}",128
""")
    print("PASS: WAIT BIOS/BDOS routing, 1/9 seconds, skipped ticks and low-word wrap")


def memory(folder):
    source = (ROOT / "mem.a86").read_text()
    body = asm(source[source.index("            mov     cx,ds"):
                      source.index("include tinylib.a86")])
    body = body.replace("es:[bx]", "[es:bx]").replace("offset mcb", "mcb")
    for version in (0x22, 0x1430, 0x1431):
        for size in (512, 640, 768):
            execute(folder, f"""
 mov dx,bdos
 mov ax,25e0h
 int 21h
 mov dx,bios
 mov ax,2512h
 int 21h
 mov ax,40h
 mov es,ax
 mov word [es:13h],{size}
 jmp payload
""" + EXIT + f"""
bdos:
 cmp cx,0ch
 je version
 cmp cx,35h
 jne failed
 mov word [mlen],1024
 iret
version:
 mov ax,{version}
 iret
bios:
 cmp word [version_number],22h
 jne failed
 mov ax,{size}
 iret
print_str:
 ret
print_uint16:
 inc byte [outputs]
 cmp byte [outputs],2
 jne number_done
 cmp ax,{min(size, 640)}
 jne failed
number_done:
 ret
quit:
 cmp byte [outputs],2
 jne failed
 jmp ok
version_number dw {version}
outputs db 0
mcb dw 0
mlen dw 0
msg_sep db 0
msg_nl db 0
payload:
""" + body)
    print("PASS: MEM 512/640/768 KB on BIOS and both CCP/M routes")


def check():
    with tempfile.TemporaryDirectory(prefix="clock-test-") as td:
        folder = Path(td)
        calendar(folder)
        rtc(folder)
        wait(folder)
        memory(folder)


if __name__ == "__main__":
    check()
