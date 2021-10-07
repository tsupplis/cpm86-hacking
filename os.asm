;
;  Licensed under the MIT license. See LICENSE file in the project root for details.
;
codeseg	segment	para public 'code'
dataseg segment para public 'data'
dataseg ends

	assume	cs:codeseg, ds:dataseg
    extrn memset_:near
    bss scb_pb_:word,257
;

	    public	setcurdrv_  
setcurdrv_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        push    cx
        push    dx
        mov 	dx,word ptr 4[bp]
        mov     cx,0Eh
        int     0E0h
        pop     dx
        pop     cx
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
setcurdrv_  	endp

	    public	getccpdrv_  
getccpdrv_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        push    cx
        push    dx
        mov     cx,0Ch
        int     0E0h
        sub     al,22H
        ja      ccpdrv_nsv
        mov     cx,31h
        int     0E0h
        add     bx, 11h
        xor     ah, ah
        mov     al, es:[bx] 
        jmp     ccpdrv_end 
ccpdrv_nsv:
        mov     al,86h
        mov     byte ptr scb_pb_,al
        mov     al,0h
        mov     byte ptr scb_pb_+1,al
        mov     dx, offset scb_pb_    
        mov     cx,31h
        int     0E0h
        xor     ah,ah
        mov     al,byte ptr scb_pb_+2
        cmp     bx,0FFFh
        jnz     ccpdrv_end
        mov     ax,255
ccpdrv_end:
        pop     dx
        pop     cx
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
getccpdrv_  	endp

	    public	getcurdrv_  
getcurdrv_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        push    cx
        push    dx
        mov     cx,19h
        int     0E0h
        pop     dx
        pop     cx
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
getcurdrv_  	endp

	    public	delay_
delay_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        push    cx
        push    dx
        mov     cx,0Ch
        int     0E0h
        sub     al,22H
        jz      delay_bios
        mov     bl, 3
        mov     al, byte ptr 4[bp]
        mul     bl
        mov     dx,ax
        ;mov     dx, word ptr 4[bp]
        mov     cx,8Dh
        int     0E0h
        jmp     delay_end
delay_bios:
        mov     ah, 00h
        int     01Ah
        add     dx, word ptr 4[bp]
        mov     bx, dx
delay_lp:
        int     01Ah
        cmp     dx, bx
        jl      delay_lp
delay_end:
        pop     dx
        pop     cx
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        xor     ax,ax
        ret
delay_	endp

	    public	bdosx_
bdosx_	proc	near
        push	bp
        mov 	bp,sp
        push	es
        push	di
        push	si
        push    bx
        push    cx
        mov 	cx,word ptr 4[bp]
        mov 	dx,word ptr 6[bp]
        int 	0E0h
        mov     cx,bx
        mov 	bx,word ptr 10[bp]
        mov     word ptr [bx],cx
        mov     cx,es
        mov 	bx,word ptr 8[bp]
        mov     word ptr [bx],cx
        pop     cx
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        and     ax,255
        ret
bdosx_	endp

	        public	osver_
osver_  	proc	near
            push	bp
            mov 	bp,sp
            push    ds
            push	es
            push	di
            push	si
            push    bx
            push    cx
            mov     cx,0Ch
            int     0E0h
            xor     ah,ah
            pop     cx
            pop     bx
            pop	    si
            pop 	di
            pop     es
            pop     ds
            pop     bp
            ret
osver_      endp

	        public	bioskey_
bioskey_	proc	near
            push	bp
            mov 	bp,sp
            push    ds
            mov     ah, 0
            int     16h
            pop     ds
            pop     bp
            ret
bioskey_    endp

	    public	pckey_
pckey_	proc	near
        push	bp
        mov 	bp,sp
        push    ds
        push    bx
        cli
        xor     ax,ax
        in      al, 60h
        in      al, 60h
        push    ax
        in      al, 61h
        mov     bl, al
        or      al, 80h
        out     61h, al
        mov     al, bl
        out     61h, al
        mov     al, 20h
        out     20h,al
        pop     ax
        sti
        pop     bx
        pop     ds
        pop     bp
        ret
pckey_  endp

codeseg	ends

	    end
