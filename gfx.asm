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

; Query only on Concurrent BDOS 31h. The original selector 30 is a no-op:
; input BX=0 then returns no capability. Save ES (v1 returns SYSDAT there).
            public gfx_ega_available_
gfx_ega_available_ proc near
            push bx
            push cx
            push dx
            push si
            push di
            push ds
            push es
            mov ax,30
            xor bx,bx
            int 0E6h
            cmp ax,4745h
            jne ega_unavailable
            mov ax,bx
            and ax,1
            jmp ega_available_done
ega_unavailable:
            xor ax,ax
ega_available_done:
            pop es
            pop ds
            pop di
            pop si
            pop dx
            pop cx
            pop bx
            ret
gfx_ega_available_ endp

; Mode 10h drawing owns the adapter while graphics is active. Original EGA
; register reads are not portable, so establish/restore known mode-0 defaults
; instead of attempting to read and restore VGA-only register state.
; All register/latch transactions run with IRQs disabled.
            public gfx_ega_clear_
gfx_ega_clear_ proc near
            pushf
            cli
            push es
            push di
            push cx
            push dx
            mov dx,03C4h
            mov ax,0F02h
            out dx,ax
            mov dx,03CEh
            mov ax,0001h
            out dx,ax
            mov ax,0003h
            out dx,ax
            mov ax,0005h
            out dx,ax
            mov ax,0FF08h
            out dx,ax
            mov ax,0A000h
            mov es,ax
            xor di,di
            xor ax,ax
            mov cx,14000
            cld
            rep stosw
            pop dx
            pop cx
            pop di
            pop es
            popf
            ret
gfx_ega_clear_ endp

; egabyte(offset, mask, color): write mode 2 expands the low four color
; bits into four planes. Reading the destination loads all four latches;
; the bit mask preserves neighboring pixels. Bit 7 requests XOR.
            public gfx_ega_byte_
gfx_ega_byte_ proc near
            push bp
            mov bp,sp
            pushf
            cli
            push es
            push bx
            push dx
            mov dx,03C4h
            mov ax,0F02h
            out dx,ax
            mov dx,03CEh
            mov ax,0205h
            out dx,ax
            mov ax,0003h
            test byte ptr 8[bp],80h
            jz ega_pixel_replace
            mov ah,18h
ega_pixel_replace:
            out dx,ax
            mov al,8
            mov ah,byte ptr 6[bp]
            out dx,ax
            mov ax,0A000h
            mov es,ax
            mov bx,4[bp]
            mov al,es:[bx]
            mov al,byte ptr 8[bp]
            and al,0Fh
            mov es:[bx],al
            mov ax,0003h
            out dx,ax
            mov ax,0005h
            out dx,ax
            mov ax,0FF08h
            out dx,ax
            pop dx
            pop bx
            pop es
            popf
            pop bp
            ret
gfx_ega_byte_ endp

; Mode 10h BIOS sets vector 43h to its 8x14 ROM font. Do not assume
; that every EGA/VGA ROM stores it at the same fixed address.
            public gfx_ega_font_
gfx_ega_font_ proc near
            push bp
            mov bp,sp
            push es
            push bx
            push cx
            push dx
            xor ax,ax
            mov es,ax
            mov bx,es:[010Ch]
            mov cx,es:[010Eh]
            mov ax,4[bp]
            mov dx,14
            mul dx
            add ax,6[bp]
            add bx,ax
            mov es,cx
            mov al,es:[bx]
            xor ah,ah
            pop dx
            pop cx
            pop bx
            pop es
            pop bp
            ret
gfx_ega_font_ endp

            public gfx_bios_palette_entry_
gfx_bios_palette_entry_ proc near
            push bp
            mov bp,sp
            push bx
            mov bl,byte ptr 4[bp]
            mov bh,byte ptr 6[bp]
            mov ax,1000h
            int 10h
            pop bx
            pop bp
            ret
gfx_bios_palette_entry_ endp

codeseg ends
    end
