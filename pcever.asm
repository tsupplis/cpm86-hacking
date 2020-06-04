                title   'PCEVER - Display Emulator Version'

include pce.asm

                cseg

                lea     si, msg_hdr
                call	print_str
	            call	pce_hook_check
	            jc	    not_pce
                lea	    di,  buffer
                mov     cx, ds
                mov     es, cx
                mov	    cx, 256
                mov	    ax, pce_hook_verstr
                call	pce_hook
	            jc	    hook_err
	            mov	    ax, pce_hook_ver
	            call	pce_hook
	            jc	    hook_err

	            lea	    si, msg_verstr1
	            call	print_str
	            lea	    si, buffer
	            call	print_str
	            lea	    si, msg_verstr2
	            call	print_str
                xchg	cx, ax
                xor	    ah, ah
                mov     bx, 16
                mov	    al, dh		
                call	print_uint16
                mov	    al, '.'
                call    print_char    
                mov	    al, dl		
                call	print_uint16
                mov	    al, '.'
                call    print_char    
                mov	    al, ch	
                call	print_uint16
	            xor	    al, al
	            lea	    si, msg_verstr3
	            call	print_str
	            call    quit

not_pce:
	            lea	    si, msg_notpce
                call	print_str
                call    quit
hook_err:
	            lea	    si, msg_hookerr
                call	print_str
                call    quit

include pcelib.asm

                dseg
                org     100h

msg_hdr         db      'Requesting PCE version ...',13,10,0
msg_notpce	    db      'Not running under PCE',13,10,0
msg_hookerr 	db      'PCE Hook Error',13,10,0
msg_verstr1	    db      'PCE version ',0
msg_verstr2	    db      ' (',0
msg_verstr3	    db      ')',13,10,0
buffer          rs      256
                db      0

                end
