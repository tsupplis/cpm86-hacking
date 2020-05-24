                title   'Skeleton'

                cseg     
                mov     dx, offset msg_hdr
                mov     cl,9            
                int     0E0h    
                jmp     quit          

quit:                
                xor     cx,cx 
                int     0E0h    
                dseg    
                org     100h
msg_hdr         db      'Test',13,10,'$'
                end

