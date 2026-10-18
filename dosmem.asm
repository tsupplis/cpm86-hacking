;*-------------------------------------------------------------------------
;  DOSMEM.ASM - DOS 2+ program-allocation probe
;
;  DOS 2+ supplies INT 21h/AH=4Ah, which reports in BX the largest allocation
;  to which the current PSP block could grow after an intentionally impossible
;  resize request.  The result therefore includes immediately contiguous free
;  paragraphs, not merely the block's current limit.
;
;  DOS 1.x has no resize service.  A missing service, any unexpected error,
;  or the exceptional successful request falls back safely to PSP:0002h.
;  The display is allocation capacity in KiB, followed by INT 12h's BIOS
;  conventional-memory total.  DOSMEM11 is the always-PSP, DOS 1.x form.
;-------------------------------------------------------------------------
program segment
        assume cs:program, ds:program
        org    100h

_start:
        clc
        ; BIOS conventional-memory total, in KiB.
        int     12h
        jc      short exit
        push    ax

        ; Save the current block size for an unsupported-service fallback.
        ; A COM program's CS is its PSP segment.
        mov     si, cs:[2]
        mov     bx, cs
        sub     si, bx

        ; AH=4Ah was introduced by DOS 2.  0FFFFh paragraphs cannot fit in
        ; conventional memory, so DOS 2+ normally fails with AX=8 and returns
        ; the maximum possible size in BX.  Start with CF set because DOS 1.x
        ; leaves flags undefined on an unimplemented dispatcher function.
        mov     ax, cs
        mov     es, ax
        mov     bx, 0FFFFh
        stc
        mov     ah, 4Ah
        int     21h
        jc      short resize_failed

        ; The request unexpectedly succeeded.  Restore the original block
        ; before reporting its size.
        mov     bx, si
        mov     ah, 4Ah
        int     21h
        mov     ax, si
        jmp     short have_allocation

resize_failed:
        cmp     ax, 8            ; insufficient memory: BX is meaningful
        jne     short use_current
        mov     ax, bx
        jmp     short have_allocation
use_current:
        mov     ax, si
have_allocation:
        mov     cl, 6
        shr     ax, cl           ; paragraphs to KiB

display:
        call    convert
        mov     ah, 2
        mov     dl, '/'
        int     21h
        pop     ax
        call    convert
exit:
        mov     ah, 09h
        mov     dx, offset eol
        int     21h
        int     20h
convert:
        mov     bl, 10
        mov     di, offset digitend
cvtloop:
        dec     di
        div     bl
        add     ah, '0'
        mov     [di], ah
        xor     ah, ah
        test    al, al
        jnz     short cvtloop
        mov     ah, 09h
        mov     dx, di
        int     21h
        ret

        db      '000000'
digitend:
        db      'K$'
eol:
        db      0Ah,0Dh,'$'
program ends
        end _start
