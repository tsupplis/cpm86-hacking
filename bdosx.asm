; :ts=8
; Copyright (C) 1983 by Manx Software Systems
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
;
bdosx_	endp
codeseg	ends
	end
