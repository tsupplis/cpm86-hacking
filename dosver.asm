
program  segment
        assume cs:program, ds:program
        org    100h
_start:     
        mov    cx,0Ch
        int    0E0h
        push   ax
        lea    dx,[_header]
        mov    ah, 09h
        int    21h
        pop    ax
        cmp    al,41h
        jb     _print_v1
        mov    ah, 30h
        int    21h
        push   ax
        xor    ah,ah
        call   _print_dec
        mov    dl, '.'
        mov    ah, 02h
        int    21h
        pop    ax
        mov    al,ah
        xor    ah,ah
        call   _print_dec
        lea    dx,[_nl]
        mov    ah, 09h
        int    21h
        jmp    _exit
_print_v1:
        lea    dx,[_v1]
        mov    ah, 09h
        int    21h
_exit:
        int    20h
_print_dec:
        mov    cx,0
        mov    dx,0
_print_dec_loop:
        cmp    ax,0
        je     _print_dec_1     
        mov    bx,10       
        div    bx                 
        push   dx             
        inc    cx             
        xor    dx,dx
        jmp    _print_dec_loop
_print_dec_1:
        cmp    cx,0
        je     _print_dec_exit
        pop    dx
        add    dx,48
        mov    ah,02h
        int    21h
        dec    cx
        jmp    _print_dec_1
_print_dec_exit:
        ret
            
_header  db    "MS-DOS Level $"
_v1      db    "1.1"
_nl      db    13, 10, "$"
program  ends
         end _start
