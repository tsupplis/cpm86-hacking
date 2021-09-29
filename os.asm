;
;  Licensed under the MIT license. See LICENSE file in the project root for details.
;
codeseg	segment	para public 'code'
	assume	cs:codeseg
;
	public	bdosx_
bdosx_	proc	near
	push	bp
	mov 	bp,sp
	push	di
	push	si
    push    bx
    push    cx
	mov 	cx,word ptr 4[bp]
	mov 	dx,word ptr 6[bp]
	int 	224
    mov     cx,bx
    mov 	bx,word ptr 10[bp]
    mov     word ptr [bx],cx
    mov 	bx,word ptr 8[bp]
    mov     cx,es
    mov     word ptr [bx],cx
    pop     cx
    pop     bx
	pop	    si
	pop 	di
	pop 	bp
    and     ax,255
	ret
bdosx_	endp

	public	bioskey_
bioskey_	proc	near
    push    ds
	push	bp
	mov 	bp,sp
    mov     ah, 0
    int     16h
    pop     bp
    pop     ds
	ret
bioskey_   endp

	public	pckey_
pckey_	proc	near
    push    ds
	push	bp
	mov 	bp,sp
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
    pop     bp
    pop     ds
	ret
pckey_   endp

codeseg	ends
	end
