;
;  Licensed under the MIT license. See LICENSE file in the project root for details.
;
codeseg	segment	para public 'code'
	assume	cs:codeseg
    
            public  gfx_bios_palette_
gfx_bios_palette_   proc    near
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
gfx_bios_palette_   endp

            public  gfx_bios_mode_
gfx_bios_mode_   proc    near
            push	bp
            mov 	bp,sp
            mov     al,byte ptr 4[bp]
            xor     ah,ah
            int     10h
            pop 	bp
            xor     ax,ax
            ret 
gfx_bios_mode_   endp

	        public	gfx_bios_pixel_
gfx_bios_pixel_	proc	near
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
gfx_bios_pixel_	endp

	        public	gfx_bios_text_
gfx_bios_text_	proc	near
            push	bp
            mov 	bp,sp
            push	si
            push    bx
            push    cx
            push    dx
            mov     ah, 02h
            mov     bh, 0
            mov     dl, byte ptr 4[bp]
            mov     dh, byte ptr 6[bp]
            int     10h
            mov     si, word ptr 8[bp]
            mov     bh, 0
            mov     bl, byte ptr 10[bp]
            or      bl, bl
            jnz     text_loop
            mov     bl, 15
text_loop:
            lodsb
            or      al, al
            jz      text_done
            mov     ah, 0Eh
            int     10h
            jmp     text_loop
text_done:
            pop     dx
            pop     cx
            pop     bx
            pop	    si
            pop 	bp
            xor     ax,ax
            ret
gfx_bios_text_	endp

; CGA memory and port primitives. No BIOS calls on this path.
            public gfx_cga_clear_
gfx_cga_clear_ proc near
            push es
            push di
            push cx
            mov ax,0B800h
            mov es,ax
            xor di,di
            xor ax,ax
            mov cx,8192
            cld
            rep stosw
            pop cx
            pop di
            pop es
            ret
gfx_cga_clear_ endp

; cgabyte(offset, mask, bits, xor_pixel)
            public gfx_cga_byte_
gfx_cga_byte_ proc near
            push bp
            mov bp,sp
            push es
            push bx
            mov ax,0B800h
            mov es,ax
            mov bx,4[bp]
            mov al,es:[bx]
            cmp word ptr 10[bp],0
            jne cga_xor
            mov ah,byte ptr 6[bp]
            not ah
            and al,ah
            or al,byte ptr 8[bp]
            jmp cga_store
cga_xor:
            xor al,byte ptr 8[bp]
cga_store:
            mov es:[bx],al
            pop bx
            pop es
            pop bp
            ret
gfx_cga_byte_ endp

            public gfx_cga_palette_
gfx_cga_palette_ proc near
            push bp
            mov bp,sp
            push dx
            mov al,byte ptr 4[bp]
            mov dx,03D9h
            out dx,al
            pop dx
            pop bp
            ret
gfx_cga_palette_ endp

; IBM PC ROM supplies the low 128 characters of the 8x8 CGA font.
            public gfx_cga_font_
gfx_cga_font_ proc near
            push bp
            mov bp,sp
            push es
            push bx
            push cx
            mov ax,0F000h
            mov es,ax
            mov bx,4[bp]
            mov cl,3
            shl bx,cl
            add bx,6[bp]
            mov al,es:0FA6Eh[bx]
            xor ah,ah
            pop cx
            pop bx
            pop es
            pop bp
            ret
gfx_cga_font_ endp

codeseg ends
    end
