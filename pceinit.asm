                title   'PCEINIT - Start CP/M'

include pce.asm

                cseg
                call    cls
                call    print_cpm_ver
                lea     si, msg_rtc
                call    print_str
                mov     cx,ds
                mov     es,cx
                mov     ax,10
	            call	pce_hook_check
	            jc	    not_pce
                mov	    ax, pce_hook_localtm
	            call	pce_hook
	            jc	    hook_err
                mov	    word ptr tm_buffer,   ax
                mov	    word ptr tm_buffer+2, bx
                mov	    word ptr tm_buffer+4, cx
                mov	    word ptr tm_buffer+6, dx
                call    print_time
                mov     cx,0Ch
                int     0E0h
                sub     al,29H
                ja      cpm31_cpm
                call    cpm_set_time
 	            jmp	    quit
cpm31_cpm:
                call    cpm31_set_time
 	            jmp	    quit

not_pce:
	            lea	    si, msg_notpce
                call	print_str
                jmp     quit
hook_err:
	            lea	    si, msg_hookerr
                call	print_str
                jmp     quit

print_time:
                push    bx
                push	si
                mov     bx, 10
                mov	    ax, word ptr tm_buffer	        ; year
                call	print_dec4
                mov	    al, '-'
                call	print_char
                mov	    al, byte ptr tm_buffer+3		; month
                inc	    ax
                call	print_dec2
                mov	    al, '-'
                call	print_char
                mov	    al, byte ptr tm_buffer+2		; day
                inc	    ax
                call	print_dec2
                mov	    al, ' '
                call	print_char
                mov	    al, byte ptr tm_buffer+5		; hour
                call	print_dec2
                mov	    al, ':'
                call	print_char
                mov	    al, byte ptr tm_buffer+4		; minute
                call	print_dec2
                mov	al, ':'
                call	print_char
                mov	    al, byte ptr tm_buffer+7		; second
                call	print_dec2
                mov	al, '.'
                call	print_char
                mov	    al, byte ptr tm_buffer+5		; centisecond
                call	print_dec2
                lea	    si, msg_nl
                call	print_str
                pop	    si
                pop     bx
                ret

leap_year:
                push    cx
                mov     cl, 2
                shr     ax, cl
                pop     cx
                ret

cpm31_set_time:
                push    es
                push    si
                push    ax
                push    bx
                push    cx
                push    dx
                mov	    ax, word ptr tm_buffer
                sub     ax, 1978
                mov     cx, 365
                mul     cx
                mov	    cl, byte ptr tm_buffer+2
                xor     ch, ch
                add     cl, 1
                add     ax, cx
                xor     dh, dh
                mov	    dl, byte ptr tm_buffer+3
                lea     cx, month_day
                add     cx, dx
                add     cx, dx
                mov     si, cx
                mov	    dx, [si]
                add     ax, dx
                cmp     dx, 59
                jnb     no_current_leap
                dec     ax
no_current_leap:
                push    ax
                mov	    ax, word ptr tm_buffer
                call    leap_year
                mov     bx, ax
                pop     ax
                add     ax, bx
                push    ax
                mov	    ax, 1978
                call    leap_year
                mov     bx, ax
                pop     ax
                sub     ax, bx
                mov	    word ptr stamp_year, ax
                mov	    al, byte ptr tm_buffer+5
                xor     ah, ah
                mov     bl, 10
                div     bl
                mov     cl, 4
                rol     al, cl
                add     al, ah
                mov     byte ptr stamp_hours, al
                mov	    al, byte ptr tm_buffer+4
                xor     ah, ah
                mov     bl, 10
                div     bl
                mov     cl, 4
                rol     al, cl
                add     al, ah
                mov     byte ptr stamp_minutes, al
                mov	    al, byte ptr tm_buffer+7
                xor     ah, ah
                mov     bl, 10
                div     bl
                mov     cl, 4
                rol     al, cl
                add     al, ah
                mov     byte ptr stamp_seconds, al
                mov     cx,ds
                mov     es,cx
                lea     dx, stamp_year
                mov     cl, 68h
                int     0E0h
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                pop     si
                pop     es
                ret

cpm_set_time:
                push    es
                push    ax
                push    bx
                push    cx
                push    dx
                cli
                mov     cl, 31h                     ;Call Get Sysdata Addrs interrupt.
                int     0E0h                        ;CP/M interrupt. ES:BX -> Sysdata.
                add     bx, 20h                     ;ES:BX now -> 17 bytes of time/date data.
                xor     ah, ah
                mov     al, byte ptr tm_buffer+3    ;Month
                inc     al
                mov     dl, 10
                div     dl
                add     al, '0'
                mov     es:[bx], al
                inc     bx
                mov     al, ah
                add     al, '0'
                mov     es:[bx], al
                inc     bx
                inc     bx
                xor     ah, ah
                mov     al, byte ptr tm_buffer+2    ;Day
                inc     al
                mov     dl, 10
                div     dl
                add     al, '0'
                mov     es:[bx], al
                inc     bx
                mov     al, ah
                add     al, '0'
                mov     es:[bx], al
                inc     bx
                inc     bx
                xor     dx, dx
                mov     ax, word ptr tm_buffer      ;Year
                mov     cx, 100
                div     cx
                mov     ax, dx
                xor     ah, ah
                mov     dl, 10
                div     dl
                add     al, '0'
                mov     es:[bx], al
                inc     bx
                mov     al, ah
                add     al, '0'
                mov     es:[bx], al
                inc     bx
                inc     bx
                xor     ah, ah
                mov     al, byte ptr tm_buffer+5    ;Hour
                mov     dl, 10
                div     dl
                add     al, '0'
                mov     es:[bx], al
                inc     bx
                mov     al, ah
                add     al, '0'
                mov     es:[bx], al
                inc     bx
                inc     bx
                xor     ah, ah
                mov     al, byte ptr tm_buffer+4    ;Min
                mov     dl, 10
                div     dl
                add     al, '0'
                mov     es:[bx], al
                inc     bx
                mov     al, ah
                add     al, '0'
                mov     es:[bx], al
                inc     bx
                inc     bx
                xor     ah, ah
                mov     al, byte ptr tm_buffer+7    ;Sec
                mov     dl, 10
                div     dl
                add     al, '0'
                mov     es:[bx], al
                inc     bx
                mov     al, ah
                add     al, '0'
                mov     es:[bx], al
                sti
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                pop     es
                ret

cls:
                push    ax
                push    bx
                push    cx
                mov     dx,27
                mov     cl,02h            
                int     0E0h    
                mov     dx,'E'
                mov     cl,02h            
                int     0E0h    
                pop     cx
                pop     bx
                pop     ax
                ret

print_cpm_ver:
                push    si
                push    ax
                push    bx
                push    cx
                push    dx
                mov     cx,0Ch
                int     0E0h
                and     ah,04h
                jz      print_ver_base
                lea     si, msg_ver_cc
                call    print_str  
print_ver_base:
                lea     si, msg_ver_hdr
                call    print_str  
                xor     ah,ah
                mov     bx,16
                mov     dl,al
                mov     cl,4
                shr     al, cl
                and     al,0Fh
                call    print_uint16
                mov     al,'.'
                call    print_char
                mov     al,dl
                and     al,0Fh
                call    print_uint16
                lea     si, msg_nl
                call    print_str  
                pop     dx
                pop     cx
                pop     bx
                pop     ax  
                pop     si
                ret


include pcelib.asm

                dseg
                org     100h

month_siz       db      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
month_day       dw      000,031,059,090,120,151,181,212,243,273,304,334
msg_notpce	    db      'Not running under PCE',13,10,0
msg_hookerr 	db      'No RTC clock',13,10,0
msg_rtc         db      'Date now: ',0
msg_ver_cc      db      'Concurrent ',0
msg_ver_hdr     db      'CP/M-86 Version ',0
msg_nl          db      13,10,0
tm_buffer       rs      256
stamp_year      rw      1
stamp_hours     rb      1
stamp_minutes   rb      1
stamp_seconds   rb      1
                db      0

                end
