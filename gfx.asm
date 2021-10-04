;
;  Licensed under the MIT license. See LICENSE file in the project root for details.
;
codeseg	segment	para public 'code'
	assume	cs:codeseg
    
            public  gfx_palette_
gfx_palette_   proc    near
            push	bp
            mov 	bp,sp
            push    bx
            push    dx
            mov     bx,word ptr 4[bp]
            xor     al,al
            mov     ah,0Bh
            int     10h
            pop     dx
            pop     bx
            pop 	bp
            xor     ax,ax
            ret 
gfx_palette_   endp

	        public	gfx_pixel_
            public  gfx_mode_
gfx_mode_   proc    near
            push	bp
            mov 	bp,sp
            mov     al,byte ptr 4[bp]
            xor     ah,ah
            int     10h
            pop 	bp
            xor     ax,ax
            ret 
gfx_mode_   endp

	        public	gfx_pixel_
gfx_pixel_	proc	near
            push	bp
            mov 	bp,sp
            push	di
            push	si
            push    bx
            push    cx
            push    dx
            mov     ah, 0ch
            mov 	al, byte ptr 8[bp]
            mov     bh, 0
            mov     cx, word ptr 4[bp]
            mov     dx, word ptr 6[bp]
            int     10h
            pop     dx
            pop     cx
            pop     bx
            pop	    si
            pop 	di
            pop 	bp
            xor     ax,ax
            ret
gfx_pixel_	endp

codeseg	ends
	end
