program     segment
            assume cs:program, ds:program
            org 100h

BOX_X       EQU 89
BOX_Y       EQU 59
BOX_W       EQU 139
BOX_H       EQU 78
BOX_T       EQU 1
BALL_W      EQU 8
BALL_H      EQU 8

_start:
            mov     ax, 0004h
            int     10h
            mov     ah, 0Bh
            mov     bh, 0
            mov     bl, 0
            int     10h
            mov     ah, 0Bh
            mov     bh, 1
            mov     bl, 1
            int     10h

flush_keys:
            mov     ah, 01h
            int     16h
            jz      top_loop_init
            mov     ah, 00h
            int     16h
            jmp     flush_keys

top_loop_init:
            mov     si, BOX_X
top_loop:
            mov     cx, si
            mov     dx, BOX_Y
            mov     al, 1
            call    putpix
            inc     si
            cmp     si, BOX_X+BOX_W
            jle     top_loop

            mov     si, BOX_X
bottom_loop:
            mov     cx, si
            mov     dx, BOX_Y+BOX_H
            mov     al, 1
            call    putpix
            inc     si
            cmp     si, BOX_X+BOX_W
            jle     bottom_loop

            mov     si, BOX_Y
left_loop:
            mov     cx, BOX_X
            mov     dx, si
            mov     al, 1
            call    putpix
            inc     si
            cmp     si, BOX_Y+BOX_H
            jle     left_loop

            mov     si, BOX_Y
right_loop:
            mov     cx, BOX_X+BOX_W
            mov     dx, si
            mov     al, 1
            call    putpix
            inc     si
            cmp     si, BOX_Y+BOX_H
            jle     right_loop

            mov     word ptr [x], BOX_X+BOX_T
            mov     word ptr [y], BOX_Y+BOX_T
            mov     word ptr [xinc], 1
            mov     word ptr [yinc], 1
            mov     byte ptr [seq], 0

main_loop:
            mov     ax, [x]
            add     ax, [xinc]
            mov     [x], ax
            mov     ax, [y]
            add     ax, [yinc]
            mov     [y], ax

            mov     ax, [x]
            cmp     ax, BOX_X+BOX_W-BALL_W-BOX_T
            jle     check_x_low
            mov     word ptr [xinc], -1
            jmp     check_y
check_x_low:
            cmp     ax, BOX_X+BOX_T
            jg      check_y
            mov     word ptr [xinc], 1

check_y:
            mov     ax, [y]
            cmp     ax, BOX_Y+BOX_H-BALL_H-BOX_T
            jle     check_y_low
            mov     word ptr [yinc], -1
            jmp     draw_ball
check_y_low:
            cmp     ax, BOX_Y+BOX_T
            jg      draw_ball
            mov     word ptr [yinc], 1

draw_ball:
            mov     al, [seq]
            xor     ah, ah
            mov     bl, 4
            div     bl
            xor     ah, ah
            mov     [frame], al

            mov     al, [frame]
            mov     cl, BALL_W*BALL_H
            mul     cl
            mov     si, offset ball_data
            add     si, ax

            xor     bx, bx
yy_loop:
            xor     di, di
xx_loop:
            mov     al, [si]
            push    ax
            mov     cx, [x]
            add     cx, di
            mov     ax, [y]
            add     ax, bx
            mov     dx, ax
            pop     ax
            call    putpix
            inc     si
            inc     di
            cmp     di, BALL_W
            jl      xx_loop
            inc     bx
            cmp     bx, BALL_H
            jl      yy_loop

            mov     al, [seq]
            inc     al
            cmp     al, 13
            jl      store_seq
            xor     al, al
store_seq:
            mov     [seq], al

            xor     ah, ah
            int     1Ah
            mov     [start_tick], dx
delay_wait:
            xor     ah, ah
            int     1Ah
            mov     ax, dx
            sub     ax, [start_tick]
            cmp     ax, 1
            jl      delay_wait

            mov     ah, 01h
            int     16h
            jnz     key_available
            jmp     main_loop
key_available:
            mov     ah, 00h
            int     16h

            mov     ax, 0003h
            int     10h
            int     20h

putpix:
            push    bx
            mov     ah, 0Ch
            mov     bh, 0
            int     10h
            pop     bx
            ret

x           dw      0
y           dw      0
xinc        dw      1
yinc        dw      1
seq         db      0
frame       db      0
start_tick  dw      0

ball_data   db      0,0,0,0,0,0,0,0
            db      0,0,1,1,1,1,0,0
            db      0,1,2,2,3,2,1,0
            db      0,1,2,2,3,3,1,0
            db      0,1,2,2,2,2,1,0
            db      0,1,2,2,2,2,1,0
            db      0,0,1,1,1,1,0,0
            db      0,0,0,0,0,0,0,0
            db      0,0,0,0,0,0,0,0
            db      0,0,1,1,1,1,0,0
            db      0,1,2,2,2,2,1,0
            db      0,1,2,2,2,2,1,0
            db      0,1,2,2,3,3,1,0
            db      0,1,2,2,3,2,1,0
            db      0,0,1,1,1,1,0,0
            db      0,0,0,0,0,0,0,0
            db      0,0,0,0,0,0,0,0
            db      0,0,1,1,1,1,0,0
            db      0,1,2,2,2,2,1,0
            db      0,1,2,2,2,2,1,0
            db      0,1,3,3,2,2,1,0
            db      0,1,2,3,2,2,1,0
            db      0,0,1,1,1,1,0,0
            db      0,0,0,0,0,0,0,0
            db      0,0,0,0,0,0,0,0
            db      0,0,1,1,1,1,0,0
            db      0,1,2,3,2,2,1,0
            db      0,1,3,3,2,2,1,0
            db      0,1,2,2,2,2,1,0
            db      0,1,2,2,2,2,1,0
            db      0,0,1,1,1,1,0,0
            db      0,0,0,0,0,0,0,0

program     ends
            end _start
