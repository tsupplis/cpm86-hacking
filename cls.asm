                title   'Clear Screen'

                cseg     
                org     100h
                mov     dx,27
                mov     cl,02h            
                int     0E0h    
                mov     dx,'E'
                mov     cl,02h            
                int     0E0h    
                jmp     quit          

quit:                
                xor     cx,cx 
                int     0E0h    
                dseg    
                end
