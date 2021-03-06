                title   'PCEEXIT - Leave Emulator '

include pce.asm

                cseg

                lea     si, msg_hdr
                call	print_str
	            call	pce_hook_check
	            jc	    not_pce
                call    delay
	            mov	    ax, pce_hook_abort
	            call	pce_hook
	            jc	    hook_err
	            xor	    al, al
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
msg_hdr         db      'Exiting PCE ...',13,10,0
msg_notpce	    db      'Not running under PCE',13,10,0
msg_hookerr 	db      'PCE Hook Error',13,10,0
                
                end
