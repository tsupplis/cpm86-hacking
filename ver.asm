                title   'CP/M Version'


                cseg  
                mov     cx,ds
                mov     es,cx
                call    print_cpm_ver   
                call    quit          

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
msg_ver_cc      db      'Concurrent ',0
msg_ver_hdr     db      'CP/M-86 Version ',0
msg_nl          db      13,10,0
                end
