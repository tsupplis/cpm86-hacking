program  segment
         assume cs:program, ds:program
            org    100h
_start:
            mov     dx, offset dos_version
            call    print_string
            mov     ah, 30h
            int     21h
            cmp     al, 1
            jge     .chk_drdos
            mov     ah, 9
            lea     dx, olddos
            int     21h
            jmp     short .exit
.chk_drdos:
            xchg    al,ah
            mov     bx,0
            call    print_ver
            mov     ax, 4452h
            int     21h
            jc      .chk_cdos
            call    fix_dr_ver
            mov     dx, offset dr_version
            call    print_string
            mov     bx,1
            call    print_ver
            jmp     .exit
.chk_cdos:
            mov     ax, 4451h
            int     21h
            jc      .exit
            call    fix_dr_ver
            mov     dx, offset cdos_version
            call    print_string
            mov     bx,1
            call    print_ver
.exit:
            lea     dx, eol
            call    print_string
            int     20h

print_string:
            push    ax
            mov     ah, 09h
            int     21h
            pop     ax
            ret

print_ver:
            push    cx
            push    dx
            push    ax
            mov     al,ah
            xor     ah,ah
            call    convert
            mov     dl, '.'
            mov     ah, 02h
            int     21h
            pop     ax
            cmp     bx,1
            je      .nz_minor
            cmp     al, 10
            jge     .nz_minor
            push    ax
            mov     ah, 2
            mov     dl, '0'
            int     21h
            pop     ax
.nz_minor:
            xor     ah, ah
            call    convert
            pop     dx
            pop     cx
            ret

convert:
            push    ax
            push    bx
            push    dx
            push    di
            mov     bl, 10
            mov     di, offset digitend
.loop:
            dec     di
            div     bl
            add     ah, '0'
            mov     [di], ah
            xor     ah, ah
            test    al, al
            jnz     short .loop
            mov     ah, 9
            mov     dx, di
            int     21h
            pop     di
            pop     dx
            pop     bx
            pop     ax
            ret

fix_dr_ver:
            push    cx
            mov     ah, al
            mov     cl,4
            shr     ah, cl
            and     ah,0Fh
            and     al,0Fh
            pop     cx
            ret

dos_version:
            db "PC/MS-DOS $"
cdos_version:
            db 0Ah,0Dh,"Concurrent BDOS $"
dr_version:
            db 0Ah,0Dh,"DR BDOS $"
olddos:
            db "1.x0$"
            db "000000"
digitend:   
            db "$"
eol:
            db 0Ah,0Dh,'$'
program  ends
         end _start
