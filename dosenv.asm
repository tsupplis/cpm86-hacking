;*-------------------------------------------------------------------------
;  DOSENV.ASM - Display the DOS process environment
;
;  DOS 2.0 introduced both INT 21h/AH=30h (Get DOS Version) and the PSP
;  environment-segment pointer at offset 2Ch.  On DOS 2+ this utility prints
;  each NUL-terminated NAME=VALUE string in that environment.  On DOS 1.x,
;  where AH=30h is absent and PSP:2Ch has no environment meaning, it prints
;  nothing and exits successfully.
;
;  The version gate intentionally examines only AL.  DOS 1.x leaves other
;  registers unspecified for an unsupported dispatcher call; AL=0 is the
;  historical no-service indication used by the PC-MODE DOS variants here.
;-------------------------------------------------------------------------
program segment
        assume cs:program, ds:program
        org     100h

_start:
        stc
        mov     ah, 30h
        int     21h
        cmp     al, 2
        jb      short exit       ; DOS 1.x: no environment to display

        ; DOS 2+ stores the environment segment in the current PSP.
        mov     ax, cs:[2Ch]
        or      ax, ax
        jz      short exit
        push    ds
        mov     ds, ax
        xor     si, si

next_string:
        cmp     byte ptr [si], 0 ; double NUL terminates the environment
        je      short done
print_character:
        lodsb
        or      al, al
        jz      short end_string
        mov     dl, al
        call    put_character
        jmp     short print_character
end_string:
        mov     dl, 13
        call    put_character
        mov     dl, 10
        call    put_character
        jmp     short next_string

done:
        pop     ds
exit:
        int     20h

put_character:
        mov     ah, 2
        int     21h
        ret

program ends
        end _start
