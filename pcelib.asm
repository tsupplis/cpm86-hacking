                cseg
pce_hook_check:
	            xor	    ax, ax
                db      0D5h
                db      000h
	            sub	    ax, 0FCEh
	            jnz	    pce_hook_check_notpce
	            ret
pce_hook_check_notpce:
	            mov	    ax, 0FFFFh
	            stc
	            ret
pce_hook:
                clc
                db      0D5h
                db      000h
                ret

quit:           
                xor     cx,cx 
                int     0E0h    

delay:
                mov     bx, 02h  
delay_l2: 
                mov     di, 0FFFFh  
delay_l1: 
                dec     di   
                jnz     delay_l1    
                dec     bx 
                jnz     delay_l2 
                ret                

print_char:
                push	ax
                push	cx
                push	dx
                push	bx
                push    es
                xor     ah, ah
                mov     dx, ax
                mov     cl, 2            
                int     0E0h    
                pop	    es
                pop	    bx
                pop	    dx
                pop	    cx
                pop	    ax
                ret
print_uint16:
                push	ax
                push	cx
                push	dx
                push	bx
                push    es
                xor	    cx, cx
print_uint16_next1:
                xor	    dx, dx
                div 	bx
                cmp	    dl, 10
                jb	    print_uint16_isdigit
                add	    dl, 'A' - 10 - '0';
print_uint16_isdigit:
                add	    dl, '0'
                push	dx
                inc	    cx
                or	    ax, ax
                jnz	    print_uint16_next1
print_uint16_next2:
                pop	    ax
                push    cx
                push    dx
                mov	    dx, ax
                xor     dh,dh
                mov	    cx, 02h
                int	    0E0h
                pop     dx
                pop     cx
                loop	print_uint16_next2
                pop     es
                pop	    bx
                pop	    dx
                pop	    cx
                pop	    ax
                ret
print_str:
                push	ax
                push	bx
                push    cx
                push    dx
                push	es
                push	si
                cld
print_str_next:
                lodsb
                or	    al, al
                jz	    print_str_done
                mov	    dx, ax
                xor     dh, dh
                mov	    cx, 02h
                int	    0E0h
                jmp	    print_str_next
print_str_done:
                pop	    si
                pop     es
                pop     dx
                pop     cx
                pop	    bx
                pop	    ax
                ret                

parse_opt:
                push	ax
                push	bx
                push	dx
                mov     bh, 1h
                cld
                mov	    ah, 1
parse_opt_skip:
                jcxz	parse_opt_done
                lodsb
                call    to_lower
                dec	    cx
                cmp	    al, 32
                jbe	    parse_opt_skip
                xor	    ah, ah
                cmp	    al, '"'
                je	    parse_opt_quote
parse_opt_next:
                cmp     al, '^'
                je      pon_nostore
                stosb
pon_nostore:
                jcxz	parse_opt_done
                lodsb
                call    to_lower
                dec	    cx
                cmp	    al, 32
                jbe	    parse_opt_done
                jmp	    parse_opt_next
parse_opt_quote:
                jcxz	parse_opt_done
                lodsb
                call    to_lower
                dec	    cx
                cmp	    al, '"'
                je	    parse_opt_done
                cmp     al, '^'
                je      parse_opt_quote
                stosb
                jmp	    parse_opt_quote

parse_opt_done:
                xor	    al, al
                stosb			
                shr	    ah, 1	
                pop	    dx
                pop	    bx
                pop	    ax
                ret
to_lower:   
                cmp     al, '^'
                jne     to_lower_cnt
                or      bh, bh
                jz      to_lower_on
                xor     bh, bh
                jmp     to_lower_end
to_lower_on:
                mov     bh, 1
                jmp     to_lower_end
to_lower_cnt:
                or      bh,  bh
                jz      to_lower_end
                cmp     al, 'A'
                jb      to_lower_end
                cmp     al, 'Z'
                ja      to_lower_end
                add     al, 32
to_lower_end:
                ret

print_dec4:
	            mov	    dx, 4
	            jmp	    print_dec

print_dec2:
            	mov	    dx, 2
	            xor	    ah, ah
print_dec:
	            push	dx
	            mov	    dx, 10
	            push	dx
	            push	ax
	            call	print_uint16
	            add	    sp, 6
	            ret
