program segment
        assume cs:program, ds:program
        org    100h
        ; Try to resize our current block to 0FFxxh paragraphs
        ; This will fail and report max paragraphs available in BX
_start:
        mov     bh, 0FFh
        mov     ah, 04Ah
        int     21h

        ; Divide by (1024/16)
        ; On 286+ we can shr ax,6 directly and save 1 byte
        xchg    ax, bx
        mov     cl, 6
        shr     ax, cl
        inc     ax

display:
        call    convert
        clc
        int     12h
        jc      short exit
        push    ax
        mov     ah, 2
        mov     dl, '/'
        int     21h
        pop     ax
        call    convert
exit:
        mov     ah, 09h
        mov     dx, offset eol
        int     21h
        mov	    ax, 4C00h
        int	    21h
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
