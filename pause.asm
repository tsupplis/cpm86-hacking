                title   'Clear Screen'

                cseg     
                mov     dx, offset msg_hdr
                mov     cl,9            
                int     0E0h    
                jmp     quit          
pause:
                mov     ah,1
                int     16h
                jz      got_key
                jmp     pause
got_key:
                mov     ah,0
                int     16h
                xor     ax,ax
                mov     cl,9
                mov     dx,offset msg_clear
                int     0E0h
                xor     cx,cx
                int     0E0h

quit:                
                xor     cx,cx 
                int     0E0h    
               
                dseg    
                org     100h
msg_hdr         db      'Press any key to continue...','$'
msg_clear       db      13,27,'K$'
                end
