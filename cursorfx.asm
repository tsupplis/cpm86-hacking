                title   'Fix Cursor'

                cseg     
                org     100h
                mov     cx,0407h
                mov     ah,01h            
                int     010h    
                xor     cx,cx 
                int     0E0h    
                dseg    
                end
