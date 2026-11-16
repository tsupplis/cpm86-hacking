#!/usr/bin/env python3
"""Execute source-derived OS wrapper routines in emu2."""
from pathlib import Path
import re
import tempfile

from testclock import EXIT, ROOT, execute


def routine(source, name):
    match = re.search(
        rf"(?ms)^{name}\s+proc\s+near\s*\n(.*?)^{name}\s+endp", source
    )
    assert match, name
    text = re.sub(r"\b(?:word|byte)\s+ptr\s+", "", match[1])
    text = re.sub(r"\b(\d+)\[bp\]", r"[bp+\1]", text)
    return name + ":\n" + text.replace("offset ", "")


def keyboard(folder, source):
    body = routine(source, "ver_mpm") + routine(source, "bioskey_")
    for version in (0x22, 0x1041, 0x1430, 0x1431, 0x1432, 0x1441, 0x0130):
        unsupported = bool((version >> 8) & 5)
        execute(folder, f"""
 mov dx,bdos
 mov ax,25e0h
 int 21h
 mov dx,bios
 mov ax,2516h
 int 21h
 mov bx,1234h
 mov si,2345h
 mov di,3456h
 mov bp,4567h
 mov ax,5678h
 mov es,ax
 mov [saved_sp],sp
 call bioskey_
 cmp ax,{0xffff if unsupported else 0x1e61}
 jne failed
 cmp byte [bios_calls],{0 if unsupported else 1}
 jne failed
 cmp bx,1234h
 jne failed
 cmp si,2345h
 jne failed
 cmp di,3456h
 jne failed
 cmp bp,4567h
 jne failed
 mov ax,es
 cmp ax,5678h
 jne failed
 mov ax,ds
 mov dx,cs
 cmp ax,dx
 jne failed
 cmp sp,[saved_sp]
 jne failed
 jmp ok
""" + EXIT + f"""
bdos:
 cmp cx,0ch
 jne failed
 mov ax,{version}
 mov bx,0bad0h
 push ax
 mov ax,0bad1h
 mov es,ax
 pop ax
 xor cx,cx
 iret
bios:
 or ah,ah
 jne failed
 inc byte [bios_calls]
 mov ax,1e61h
 iret
saved_sp dw 0
bios_calls db 0
""" + body)
    print("PASS: BIOS keyboard routing on seven OS types/versions and register preservation")


def chain(folder, source):
    body = routine(source, "p_chain_")
    for submode in (0, 1):
        execute(folder, f"""
 mov dx,bdos
 mov ax,25e0h
 int 21h
 mov [saved_sp],sp
 mov ax,{submode}
 push ax
 mov ax,command
 push ax
 mov bx,1234h
 mov si,2345h
 mov di,3456h
 mov bp,4567h
 mov ax,5678h
 mov es,ax
 call p_chain_
 add sp,4
 cmp bx,1234h
 jne failed
 cmp si,2345h
 jne failed
 cmp di,3456h
 jne failed
 cmp bp,4567h
 jne failed
 mov ax,es
 cmp ax,5678h
 jne failed
 cmp sp,[saved_sp]
 jne failed
 cmp byte [sub_calls],{submode}
 jne failed
 cmp byte [chain_calls],1
 jne failed
 mov si,chainbuf
 mov di,expected
 mov cx,10
 cld
 push ds
 pop es
 repe cmpsb
 jne failed
 jmp ok
""" + EXIT + """
bdos:
 mov bx,0bad0h
 cmp cx,33h
 je dma_segment
 cmp cx,1ah
 je dma_offset
 cmp cx,2fh
 jne failed
 or dx,dx
 jne failed
 inc byte [chain_calls]
 mov ax,0ffffh
 iret
dma_segment:
 mov ax,ds
 cmp dx,ax
 jne failed
 iret
dma_offset:
 cmp dx,chainbuf
 jne failed
 iret
setsub:
 mov bx,0bad1h
 inc byte [sub_calls]
 ret
saved_sp dw 0
sub_calls db 0
chain_calls db 0
command db "b:foo arg",0
expected db "B:FOO ARG",0
chainbuf times 128 db 0
""" + body)
    print("PASS: failed P_CHAIN preserves C registers/stack with both submit modes")


def check():
    source = (ROOT / "os.asm").read_text()
    with tempfile.TemporaryDirectory(prefix="asm-test-") as td:
        folder = Path(td)
        keyboard(folder, source)
        chain(folder, source)


if __name__ == "__main__":
    check()
