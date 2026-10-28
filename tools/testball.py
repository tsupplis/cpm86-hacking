#!/usr/bin/env python3
"""Run the built BALL.CMD under synthetic BDOS/BIOS in emu2.

Checks OS routing, two animated frames, packed pixels, text restoration,
exit and register/segment preservation across deliberately hostile BDOS calls.
"""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent.parent
HARNESS = r'''bits 16
cpu 8086
org 100h
start:
    mov dx,bdos
    mov ax,25e0h
    int 21h
    mov dx,video
    mov ax,2510h
    int 21h
    mov dx,timer
    mov ax,251ah
    int 21h
    mov dx,forbidden
    mov ax,2516h
    int 21h
    mov ax,cs
    add ax,(payload-$$+100h)/16
    mov [entry+2],ax
    mov ds,ax
    mov es,ax
    jmp far [cs:entry]
bdos:
    cmp cx,12
    je version
    cmp cx,6
    je keyboard
    cmp cx,141
    je delay
    cmp cx,9
    je string
    cmp cx,2
    je output
    cmp cx,0
    je done
    jmp forbidden
version:
    mov ax,VERSION
    jmp trash
keyboard:
    inc byte [cs:polls]
    xor ax,ax
    cmp byte [cs:polls],3
    jb trash
    mov al,'X'
    jmp trash
delay:
%if !KERNEL
    jmp forbidden
%endif
    cmp dx,3
    jne forbidden
    inc byte [cs:delays]
    jmp trash
string:
%if UNSUPPORTED
    inc byte [cs:messages]
%endif
    jmp trash
output:
%if !KERNEL
    jmp forbidden
%endif
    mov bx,[cs:outpos]
    mov [cs:bytes+bx],dl
    inc word [cs:outpos]
trash:
    ; No dependence on scratch registers or ES/DS surviving BDOS.
    mov bx,0aaaaH
    mov cx,bx
    mov dx,bx
    mov si,bx
    mov di,bx
    mov bp,bx
    mov es,bx
    mov ds,bx
    iret
video:
%if KERNEL || UNSUPPORTED
    jmp forbidden
%endif
    cmp ax,4
    je mode4
    cmp ax,3
    je mode3
    cmp ah,0bh
    jne forbidden
    iret
mode4:
    inc byte [cs:entered]
    push es
    push ax
    push cx
    push di
    mov ax,0b800h
    mov es,ax
    xor ax,ax
    xor di,di
    mov cx,8192
    rep stosw
    pop di
    pop cx
    pop ax
    pop es
    iret
mode3:
    inc byte [cs:left]
    iret
timer:
%if KERNEL || UNSUPPORTED
    jmp forbidden
%endif
    inc word [cs:ticks]
    mov dx,[cs:ticks]
    xor cx,cx
    iret
forbidden:
    mov ax,4c01h
    int 21h
done:
    cli
    mov ax,cs
    mov ss,ax
    mov sp,stack_end
    mov ds,ax
    sti
%if UNSUPPORTED
    cmp byte [messages],1
    jne forbidden
    cmp byte [polls],0
    jne forbidden
%else
    cmp byte [polls],3
    jne forbidden
%if KERNEL
    cmp byte [delays],2
    jne forbidden
%if (VERSION & 0ffh) == 31h
    mov si,expected31
    mov cx,16
%else
    mov si,expected
    mov cx,6
%endif
    cmp [outpos],cx
    jne forbidden
    mov di,bytes
    push ds
    pop es
    repe cmpsb
    jne forbidden
%else
    cmp byte [entered],1
    jne forbidden
    cmp byte [left],1
    jne forbidden
%endif
    mov ax,0b800h
    mov es,ax
    ; Border: x=89..228, y=59. Neighbours remain black.
    xor di,di
    mov cx,320
border:
    xor al,al
    cmp cx,89
    jb border_black
    cmp cx,228
    ja border_black
    mov al,1
border_black:
    ; Check the whole top scanline, including adjacent packed pixels.
    push cx
    push ax
    mov bx,cx
    shr bx,1
    shr bx,1
    mov dl,[es:8192+29*80+bx]
    and cl,3
    xor cl,3
    shl cl,1
    shr dl,cl
    and dl,3
    pop ax
    pop cx
    cmp dl,al
    jne forbidden
    loop border
    ; Frame zero at second position (92,62); row 1 columns 2..5 = 1.
    cmp byte [es:8192+31*80+23],5
    jne forbidden
    cmp byte [es:8192+31*80+24],50h
    jne forbidden
%endif
    mov ax,4c00h
    int 21h
entry: dw 100h,0
polls db 0
delays db 0
messages db 0
entered db 0
left db 0
ticks dw 0
outpos dw 0
bytes times 32 db 0
expected db 27,'a',4,27,'a',3
expected31 db 27,'a',4,27,'!',30,0,0,0,20h,2,0,0,27,'a',3
times 256 db 0
stack_end:
align 16
payload:
incbin IMAGE,128
'''


def check():
    cases = ((0x0022, 0, 0), (0x1431, 1, 0), (0x1432, 1, 0),
             (0x1441, 1, 0), (0x1041, 1, 0), (0x1450, 1, 0),
             (0x1460, 0, 0), (0x1030, 0, 1), (0x1031, 0, 1))
    with tempfile.TemporaryDirectory(prefix='ball-test-') as td:
        source = Path(td) / 'balltest.asm'
        image = Path(td) / 'balltest.com'
        source.write_text(HARNESS)
        for version, kernel, unsupported in cases:
            subprocess.run(['nasm', '-f', 'bin', str(source), '-o', str(image),
                            f'-DIMAGE="{ROOT / "ball.cmd"}"', f'-DVERSION={version}',
                            f'-DKERNEL={kernel}', f'-DUNSUPPORTED={unsupported}'], check=True)
            result = subprocess.run(['emu2', str(image)], capture_output=True, timeout=15)
            assert result.returncode == 0, (hex(version), result.stdout, result.stderr)
    print(f'PASS: {len(cases)} BALL OS routes, animation, pixels, input, delay and cleanup')


if __name__ == '__main__':
    check()
