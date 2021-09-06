                title   'CP/M-86 TOD replacement'

                cseg

                mov     cx,ds
                mov     es,cx
                mov     cx,0Ch
                int     0E0h
                sub     al,22H
                ja      wrong_os
                call    arg_avail
                or      ax,ax
                jz      just_show
                call    parse_date
                or      ax,ax
                jnz     wrong_fmt
                call    check_date
                or      ax,ax
                jnz     wrong_fmt
                call    cpm_set_time
                cmp     ax,0
                jnz     wrong_fmt
just_show:  
                call    clear_keys
just_show_time:
                call    cpm_show_time
                mov     cl, byte ptr disp_loop
                or      cl, 0
                jz      loop_just_show
 	            call    quit
loop_just_show:  
                call    wait_key
                or      ax, 0
                jnz     loop_show_done
	            lea	    si, msg_back
                call	print_str
                jmp     just_show_time
loop_show_done:
                call    clear_keys
                call    quit
wrong_os:
	            lea	    si, msg_wrong_os
                call	print_str
                call    quit
wrong_fmt:
	            lea	    si, msg_wrong_fmt
                call	print_str
                call    quit

arg_avail:
                push    cx
                push    si
                mov     si, 80h
                mov     cl, byte ptr [si]
                or      cl,cl
                jz      arg_noarg
tail_loop:
                inc     si
                mov     al, ds:[si]
                cmp     al,' '
                je      tail_loop
                cmp     al,0Dh
                je      arg_noarg
                cmp     al,00h
                je      arg_noarg
arg_args:
                cmp     al, 'P'
                je      arg_loop
                mov     ax,1
                jmp     arg_end
arg_loop:       
                mov     al, 0
                mov     byte ptr disp_loop, al
arg_noarg:
                mov     ax,0
                jmp     arg_end
arg_end:
                pop     si
                pop     cx
                ret

parse_date:
                push    es
                push    bx 
                push    dx
                push    di
                mov     cx,ds
                mov     es,cx
 
                mov     di, 80h          ; date
                lea     si, tmz_date
                
pd_spc:
                inc     di
                mov     al, [di] 
                cmp     al, ' '
                je      pd_spc

                mov     cx, 2
pd_year:
                mov     al, [di]            ; year
                call    isdigit
                or      ah, ah
                jnz     pd_err
                mov     [si], al
                inc     si
                inc     di
                dec     cx
                or      cx,cx
                jnz     pd_year

                mov     al, [di]
                cmp     al, '/'
                jne     pd_err
                inc     si
                inc     di

                mov     cx, 2
pd_month:
                mov     al, [di]            ; month
                call    isdigit
                or      ah, ah
                jnz     pd_err
                mov     [si], al
                inc     si
                inc     di
                dec     cx
                or      cx,cx
                jnz     pd_month

                mov     al, [di]
                cmp     al, '/'
                jne     pd_err
                inc     si
                inc     di

                mov     cx, 2
pd_day:
                mov     al, [di]            ; day
                call    isdigit
                or      ah, ah
                jnz     pd_err
                mov     [si], al
                inc     si
                inc     di
                dec     cx
                or      cx,cx
                jnz     pd_day
                
                inc     si
                jnz     pt_spc
pd_err:
                jmp     pt_err

pt_section:
                inc     di
pt_spc:
                mov     al, [di] 
                cmp     al, ' '
                je      pt_section

                mov     cx, 2
pt_hour:
                mov     al, [di]            ; hour
                call    isdigit
                or      ah, ah
                jnz     pt_err
                mov     [si], al
                inc     si
                inc     di
                dec     cx
                or      cx,cx
                jnz     pt_hour

                mov     al, [di]
                cmp     al, ':'
                jne     pt_err
                inc     si
                inc     di

                mov     cx, 2
pt_min:
                mov     al, [di]            ; minute
                call    isdigit
                or      ah, ah
                jnz     pt_err
                mov     [si], al
                inc     si
                inc     di
                dec     cx
                or      cx,cx
                jnz     pt_min

                mov     al, [di]
                cmp     al, ':'
                jne     pt_err
                inc     si
                inc     di

                mov     cx, 2
pt_sec:
                mov     al, [di]            ; second
                call    isdigit
                or      ah, ah
                jnz     pt_err
                mov     [si], al
                inc     si
                inc     di
                dec     cx
                or      cx,cx
                jnz     pt_sec

                mov     ax,0
                jmp     pt_end
pt_err:
                mov     ax,1
                jmp     pt_end
pt_end:
                pop     di
                pop     dx
                pop     bx
                pop     es
                ret

check_date:
                push    bx
                push    cx
                push    dx

                mov     al, byte ptr tmz_date+6   ;check leap year
                sub     al, '0'
                xor     ah,ah
                mov     bl,10
                imul    bl
                xor     bh, bh
                mov     bl, byte ptr tmz_date+7   
                sub     bl, '0'
                add     ax, bx
                or      ax, ax
                jz      no_leap
                and     ax, 3
                jnz     no_leap
                mov     bx, offset month_siz+2
                mov     ax, 29
                mov     [bx], ax
no_leap:
                mov     al, byte ptr tmz_date   ;check month
                sub     al, '0'
                xor     ah,ah
                mov     bl,10
                imul    bl
                xor     bh, bh
                mov     bl, byte ptr tmz_date+1   
                sub     bl, '0'
                add     ax, bx
                mov     dx, ax
                cmp     ax, 12
                ja      ckd_err
                cmp     ax, 1
                jb      ckd_err

                mov     al, byte ptr tmz_date+3   ;check day
                sub     al, '0'
                xor     ah,ah
                mov     bl,10
                imul    bl
                xor     bh, bh
                mov     bl, byte ptr tmz_date+4  
                sub     bl, '0'
                add     ax, bx
                mov     bx, offset month_siz
                add     bx,dx
                mov     cl, [bx]
                xor     ch, ch
                cmp     ax, cx
                ja      ckd_err
                cmp     ax, 1
                jb      ckd_err
                jmp     ck_time

ckd_err:
                jmp     ckt_err
ck_time:
                mov     al, byte ptr tmz_time   ;check hour
                sub     al, '0'
                xor     ah,ah
                mov     bl,10
                imul    bl
                xor     bh, bh
                mov     bl, byte ptr tmz_time+1   
                sub     bl, '0'
                add     ax, bx
                cmp     ax, 23
                ja      ckt_err

                mov     al, byte ptr tmz_time+3   ;check minutes
                sub     al, '0'
                xor     ah,ah
                mov     bl,10
                imul    bl
                xor     bh, bh
                mov     bl, byte ptr tmz_time+4  
                sub     bl, '0'
                add     ax, bx
                cmp     ax, 59
                ja      ckt_err

                mov     al, byte ptr tmz_time+6   ;check seconds
                sub     al, '0'
                xor     ah,ah
                mov     bl,10
                imul    bl
                xor     bh, bh
                mov     bl, byte ptr tmz_time+7 
                sub     bl, '0'
                add     ax, bx
                cmp     ax, 59
                ja      ckt_err

                mov     ax,0
                jmp     ckt_end
ckt_err:
                mov     ax,1
                jmp     ckt_end
ckt_end:
                pop     dx
                pop     cx
                pop     bx
                ret

cpm_set_time:
                push    es
                push    ds
                push    si
                push    di
                push    bx
                push    cx
                push    dx
                cli

                mov     cl, 31h                     ;Call Get Sysdata Addrs interrupt.
                int     0E0h                        ;CP/M interrupt. ES:BX -> Sysdata.
                add     bx, 20h                     ;ES:BX now -> 17 bytes of time/date data.
                mov     di, bx
                mov     byte ptr tmz_sep,','
                lea     si, tmz_date
                mov     cx, 17
cst_cpy:        mov     bl, ds:[si]     ;copy source to destination
                mov     es:[di], bl
                inc     si              ;increment source and destination
                inc     di	
                dec     cx              ;decrement count
                jnz     cst_cpy         ;if not zero goto next bit
                sti
                mov     ax, 0
                pop     dx
                pop     cx
                pop     bx
                pop     di
                pop     si
                pop     ds
                pop     es
                ret

cpm_show_time:
                push    es
                push    ds
                push    si
                push    di
                push    ax
                push    bx
                push    cx
                push    dx
                cli

                mov     cl, 31h                     ;Call Get Sysdata Addrs interrupt.
                int     0E0h                        ;CP/M interrupt. ES:BX -> Sysdata.
                add     bx, 20h                     ;ES:BX now -> 17 bytes of time/date data.
                mov     di, bx
                lea     si, tmz_date
                mov     cx, 17
csh_cpy:        mov     bl, es:[di]     ;copy source to destination
                mov     ds:[si], bl
                inc     si              ;increment source and destination
                inc     di	
                dec     cx              ;decrement count
                jnz     csh_cpy         ;if not zero goto next bit
                sti
                lea     si, tmz_date
                mov     byte ptr tmz_sep,9
                call    print_str
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                pop     di
                pop     si
                pop     ds
                pop     es
                ret

isdigit:
                cmp     al, '0'
                jb      id_err
                cmp     al, '9'
                ja      id_err
                mov     ah, 0
                jmp     id_end
id_err:
                mov     ah, 1
id_end:
                ret

include pcelib.asm

                dseg
                org     100h
month_siz       db      0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
msg_wrong_os    db      'Requires CP/M-86 1.1',13,10,0
msg_wrong_fmt   db      'Invalid Date & Time Format',13,10
                db      'Please retry using:',13,10
                db      'X>TOD MM/DD/YY HH:MM:SS',13,10
                db      'where YY is in range 00-99 for 2000 to 2099',13,10,0
msg_nl          db      13,10,0
msg_back        db      8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
disp_loop       db      1
tmz_date        db      '01/01/01'
tmz_sep         db      9
tmz_time        db      '01:01:01'
                db      0
tm_buffer       rs      256
                db      0

                end
