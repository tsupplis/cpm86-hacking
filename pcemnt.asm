                title   'PCEMNT - Mount Image File'

include pce.asm

                cseg

                mov     cx, ds
                mov     es, cx
                lea     si, msg_hdr
                call	print_str
                mov     ax, 10
	            call	pce_hook_check
	            jc	    not_pce  
                mov	    si, 80h		
                lodsb				
                mov	    cl, al
                xor	    ch, ch
                lea	    di, str1
                call	parse_opt	
                jc	    usage
                lea	    di, str2
                call	parse_opt		
                lea	    di, str1
                jc	    insert

id:
	            lea	    si, msg_id
                call    print_str
                mov     al,' '
                call    print_char
	            mov	    si, di
                call    print_str
	            lea	    si, msg_id
	            mov	    ax, pce_hook_setmsg
	            call	pce_hook		
	            jc	    hook_err
	            lea	    di, str2
                mov     al,' '
                call    print_char
insert:
	            lea	    si, msg_insert
                call    print_str
                mov     al,' '
                call    print_char
	            mov	    si, di
                call    print_str
	            lea	    si, msg_conclude
                call    print_str
	            lea	    si, msg_insert
	            mov	    ax, pce_hook_setmsg
	            call	pce_hook		
	            jc	    hook_err
	            jmp	    quit

usage:
	            lea	    si, msg_usage
	            call	print_str
	            jmp	    quit

not_pce:
	            lea	    si, msg_notpce
                call	print_str
                jmp     quit
hook_err:
	            lea	    si, msg_hookerr
                call	print_str
                jmp     quit

include pcelib.asm

                dseg
                org     100h

msg_id		    db      'disk.id',0
msg_insert	    db      'disk.insert',0
msg_usage	    db      'usage: pcemnt [drive] [filename]',13,10,0
msg_hdr         db      'Requesting PCE mount ...',13,10,0
msg_notpce	    db      'Not running under PCE',13,10,0
msg_hookerr 	db      'PCE Hook Error',13,10,0
msg_conclude 	db      ' ...',13,10,0
str1            rs      256
str2            rs      256
                db      0

                end
