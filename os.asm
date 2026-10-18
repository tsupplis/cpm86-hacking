;
;  Licensed under the MIT license. See LICENSE file in the project root for details.
;
codeseg	segment	para public 'code'
dataseg segment para public 'data'
dataseg ends

	assume	cs:codeseg, ds:dataseg
    extrn memset_:near
    bss scb_pb_:word,257
    bss chainbuf:byte,128
    bss clibuf:byte,130
;

;
; BDOS version gates for the calls below. Each one runs BDOS fn 12
; (AH = system type, AL = version) and returns with CF set (JB) when the
; running OS does not support the call. Clobbers AX,BX,CX,DX,ES.
;
;  ver_ge30: version >= 30h (MP/M II, CP/M 3, CCP/M 3.x, DOS Plus ...)
;            CP/M-86 1.1 returns 22h, so it is rejected.
;  ver_ge41: version >= 41h (CP/M-86 v4 / DOS Plus)
;
ver_ge30	proc	near
 mov	cx,0Ch
 int	0E0h
 cmp	al,30h
 ret
ver_ge30	endp

;  ver_gt22: version > 22h (T_GET/T_SET: MP/M II, DOS+ 2.x and later);
;            CF set when unsupported.
ver_gt22	proc	near
	mov	cx,0Ch
	int	0E0h
	cmp	al,23h
	ret
ver_gt22	endp

;  ver_cpm3: version >= 30h and not MP/M II (system type bit 0 clear),
;            i.e. CP/M 3 and above; CF set when unsupported.
ver_cpm3	proc	near
	mov	cx,0Ch
	int	0E0h
	cmp	al,30h
	jb	ver_cpm3_ret		; CF set
	test	ah,01h			; CF cleared
	jz	ver_cpm3_ret
	stc
ver_cpm3_ret:
	ret
ver_cpm3	endp

;  ver_mpm:  MP/M or multi-user system (system type bit 0 or 2 set), i.e.
;            MP/M, Concurrent CP/M, CCP/M-86; CF set when unsupported.
;  ver_mpm4: ver_mpm, or CP/M-86 v4 (version >= 41h); CF set when unsupported.
ver_mpm	proc	near
	mov	cx,0Ch
	int	0E0h
	test	ah,05h			; CF cleared
	jnz	ver_mpm_ret
	stc
ver_mpm_ret:
	ret
ver_mpm	endp

ver_mpm4	proc	near
	mov	cx,0Ch
	int	0E0h
	test	ah,05h			; CF cleared
	jnz	ver_mpm4_ret
	cmp	al,41h			; CF set when below
ver_mpm4_ret:
	ret
ver_mpm4	endp

ver_ge41	proc	near
 mov	cx,0Ch
 int	0E0h
 cmp	al,41h
 ret
ver_ge41	endp

;
; getftime: BDOS fn 102 (66h, F_TIMEDATE) -- get the file date/time stamps
; and password mode of an open FCB: create/access stamp at FCB+18h, update
; stamp at FCB+1Ch, password mode at FCB+0Ch. MP/M II and later (version
; >= 30h).
;
; int getftime(char *fcb)  -- 0..3 = ok, 0xFF = failed or not supported
;
     public	getftime_
getftime_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        call    ver_ge30
        jb      getftime_no
        mov 	dx,word ptr 4[bp]
        mov     cx,66h
        int     0E0h
        xor     ah,ah
        jmp     getftime_end
getftime_no:
        mov     ax,0FFh
getftime_end:
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
getftime_	endp

;
; setftime: BDOS fn 116 (74h) -- set the file date/time stamps. CP/M-86 v4
; only (version >= 41h). The stamps are read from the DMA address, so the
; DMA address is pointed at 'stamps' for the call and the previous DMA
; address (BDOS fn 52) is restored afterwards. 'stamps' uses the same
; layout as getftime returns at FCB+18h: create/access stamp (4 bytes:
; DW days since 1/1/78, DB BCD hour, DB BCD minute) followed by the update
; stamp (4 bytes, same layout).
;
; int setftime(char *fcb, char *stamps)  -- 0..3 = ok, 0xFF = failed or
;                                           not supported
;
     public	setftime_
setftime_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        call    ver_ge41
        jb      setftime_no
        mov     cx,34h                  ; get DMA address -> ES:BX
        int     0E0h
        push    es                      ; -10[bp] old DMA segment
        push    bx                      ; -12[bp] old DMA offset
        push    ax                      ; -14[bp] result slot
        mov     dx,ds
        mov     cx,33h                  ; DMA segment = DS
        int     0E0h
        mov     dx,word ptr 6[bp]
        mov     cx,1Ah                  ; DMA offset = stamps
        int     0E0h
        mov     dx,word ptr 4[bp]
        mov     cx,74h                  ; set file date & time
        int     0E0h
        xor     ah,ah
        mov     word ptr -14[bp],ax
        mov     dx,word ptr -10[bp]
        mov     cx,33h                  ; restore DMA segment
        int     0E0h
        mov     dx,word ptr -12[bp]
        mov     cx,1Ah                  ; restore DMA offset
        int     0E0h
        mov     ax,word ptr -14[bp]
        jmp     setftime_end
setftime_no:
        mov     ax,0FFh
setftime_end:
        mov     sp,bp
        sub     sp,8
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
setftime_	endp

;
; gettime / settime: BDOS fn 105 (69h, T_GET) / fn 104 (68h, T_SET) -- the
; system date and time, MP/M II and later (version > 22h). The stamp is the
; same 4-byte layout getftime/setftime use in the FCB and in 'stamps':
;   DW  day     ; day 1 = 1 January 1978, little endian
;   DB  hour    ; packed BCD
;   DB  minute  ; packed BCD
;   DB  second  ; packed BCD, DOS Plus only: T_GET stores it and T_SET reads it
; so the buffer must be 5 bytes. Elsewhere the seconds are not in the stamp:
; T_GET returns them in AL, and T_SET resets them to zero.
;
; int gettime(char *stamp)  -- seconds (packed BCD, 0..59h) or -1 if unsupported
; int settime(char *stamp)  -- 0 or -1 if unsupported
;
	    public	gettime_
gettime_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        call    ver_gt22
        jb      time_no
        mov 	dx,word ptr 4[bp]
        mov     cx,69h
        int     0E0h
        xor     ah,ah
        jmp     time_end
time_no:
        mov     ax,-1
time_end:
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
gettime_	endp

	    public	settime_
settime_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        call    ver_gt22
        jb      settime_no
        mov 	dx,word ptr 4[bp]
        mov     cx,68h
        int     0E0h
        xor     ax,ax
        jmp     settime_end
settime_no:
        mov     ax,-1
settime_end:
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
settime_	endp

;
; getpcode / setpcode: BDOS fn 108 (6Ch, P_CODE) -- get/put the program
; return code. getcmode / setcmode: BDOS fn 109 (6Dh, C_MODE) -- get/set the
; console mode bitmap (CP/M 3: bit 0 fn 11 sees only ^C, 1 no ^S pause,
; 2 no tab expansion/^P echo, 3 ^C does not abort, 4 like 2, 7 CCP/M ^O,
; 8-9 GET/SUBMIT fn 11 behaviour; Personal CP/M: bit 4 only).
; CP/M 3 and above: version >= 30h, not MP/M II (system type bit 0 set).
; All return the value in HL, or 0FFFFh when the OS does not support the
; call (not a valid return code). set*(0xFFFF) is a read.
;
; unsigned getpcode(void)       unsigned setpcode(unsigned code)
; unsigned getcmode(void)       unsigned setcmode(unsigned mode)
;
	    public	getpcode_
getpcode_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        mov     dx,0FFFFh
        mov     cx,6Ch
        jmp     cpm3_go
getpcode_	endp

	    public	setpcode_
setpcode_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        mov 	dx,word ptr 4[bp]
        mov     cx,6Ch
        jmp     cpm3_go
setpcode_	endp

	    public	getcmode_
getcmode_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        mov     dx,0FFFFh
        mov     cx,6Dh
        jmp     cpm3_go
getcmode_	endp

	    public	setcmode_
setcmode_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        mov 	dx,word ptr 4[bp]
        mov     cx,6Dh
cpm3_go:
        push    cx
        push    dx
        call    ver_cpm3
        pop     dx
        pop     cx
        jb      cpm3_no
        int     0E0h
        mov     ax,bx
        jmp     cpm3_end
cpm3_no:
        mov     ax,0FFFFh
cpm3_end:
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
setcmode_	endp

;
; p_cli: BDOS fn 150 (96h, P_CLI) -- send a command line to the CLI
; (resident system process or program load). MP/M and Concurrent CP/M only:
; system type bit 0 (MP/M) or bit 2 (multi-user) set. The command is copied
; (max 127 chars) into a static 130-byte CLI buffer
;   DB 0 / DS 128 (0-terminated command line) / DB 0
;
; int p_cli(char *cmd)  -- 0 = ok, 0xFF = failed or not supported
;
     public	p_cli_
p_cli_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        call    ver_mpm
        jb      p_cli_no
        push    ds
        pop     es
        mov     si,word ptr 4[bp]
        mov     di,offset clibuf
        mov     byte ptr [di],0		; reserved
        inc     di
        mov     cx,127
p_cli_copy:
        lodsb
        or      al,al
        jz      p_cli_done
        stosb
        loop    p_cli_copy
p_cli_done:
        mov     byte ptr [di],0
        mov     byte ptr clibuf+129,0	; reserved
        mov     dx,offset clibuf
        mov     cx,96h
        int     0E0h
        xor     ah,ah
        jmp     p_cli_end
p_cli_no:
        mov     ax,0FFh
p_cli_end:
        pop     bx
        pop	si
        pop 	di
        pop 	es
        pop 	bp
        ret
p_cli_	endp

;
; p_priority: BDOS fn 145 (91h, P_PRIORITY) -- set the process priority.
; MP/M, Concurrent CP/M (ver_mpm). 0 is the highest priority, 0FFh the
; lowest; transient programs start at 200.
;
; int p_priority(int priority)  -- 0 = ok, -1 = not supported
;
     public	p_priority_
p_priority_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        call    ver_mpm
        jb      prio_no
        mov 	dx,word ptr 4[bp]
        mov     cx,91h
        int     0E0h
        xor     ax,ax
        jmp     prio_end
prio_no:
        mov     ax,-1
prio_end:
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
p_priority_	endp

;
; conattach: BDOS fn 146 (92h, C_ATTACH) -- block until the process's default
; console is available and attach to it. MP/M, Concurrent CP/M (ver_mpm).
; Done automatically by any console I/O while detached.
;
; int conattach(void)  -- 0 = ok, -1 = not supported
;
     public	conattach_
conattach_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        call    ver_mpm
        jb      attach_no
        xor     dx,dx
        mov     cx,92h
        int     0E0h
        xor     ax,ax
        jmp     attach_end
attach_no:
        mov     ax,-1
attach_end:
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
conattach_	endp

;
; condetach: BDOS fn 147 (93h, C_DETACH) -- disconnect the program from the
; screen and keyboard; it must run in the background. MP/M, Concurrent CP/M
; and CP/M-86 v4 (ver_mpm4). Under CP/M-86 v4 using the keyboard while
; detached terminates the program, and only INT 0E0h calls may be used (no
; INT 2nh DOS-like calls). Under MP/M and CCP/M screen output re-attaches.
;
; int condetach(void)  -- 0 = ok, -1 = not supported
;
     public	condetach_
condetach_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        call    ver_mpm4
        jb      detach_no
        xor     dx,dx
        mov     cx,93h
        int     0E0h
        xor     ax,ax
        jmp     detach_end
detach_no:
        mov     ax,-1
detach_end:
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
condetach_	endp

;
; getpid / getppid / getpriority / getppriority: read the process descriptor
; (PD) returned by BDOS fn 156 (9Ch, P_PDADR) in ES:BX -- see
; commands/submit/submit.a86 in ccpm86-y2k. MP/M, Concurrent CP/M (ver_mpm).
; PD offsets used:  +05h p_prior (priority byte)
;                   +1Eh p_parent (parent PD offset in the same segment, 0 = none)
; The process ID is the PD address (offset in the system data segment).
;
; unsigned getpid(void)   -- PD address of this process, 0FFFFh if unsupported
; unsigned getppid(void)  -- PD address of the parent, 0 if none, 0FFFFh if unsupported
; int getpriority(void)   -- priority 0..255 of this process, -1 if unsupported
; int getppriority(void)  -- priority 0..255 of the parent, -1 if no parent or
;                            unsupported
;
pd_get	proc	near
	call	ver_mpm
	jb	pd_get_ret		; CF set: unsupported
	mov	cx,9Ch
	int	0E0h			; ES:BX = process descriptor
	clc
pd_get_ret:
	ret
pd_get	endp

     public	getpid_
getpid_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        mov     dx,0
        jmp     pd_go
getpid_	endp

     public	getppid_
getppid_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        mov     dx,1
        jmp     pd_go
getppid_	endp

     public	getpriority_
getpriority_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        mov     dx,2
        jmp     pd_go
getpriority_	endp

     public	getppriority_
getppriority_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push	di
        push	si
        push    bx
        mov     dx,3
pd_go:
        push    dx
        call    pd_get
        pop     dx
        jb      pd_no
        cmp     dx,1
        jb      pd_pid
        je      pd_ppid
        cmp     dx,2
        je      pd_prio
        mov     bx,word ptr es:1Eh[bx]	; parent PD
        or      bx,bx
        jz      pd_no			; no parent
pd_prio:
        xor     ax,ax
        mov     al,byte ptr es:5[bx]	; p_prior
        jmp     pd_end
pd_ppid:
        mov     ax,word ptr es:1Eh[bx]	; p_parent
        jmp     pd_end
pd_pid:
        mov     ax,bx
        jmp     pd_end
pd_no:
        mov     ax,-1
pd_end:
        pop     bx
        pop	    si
        pop 	di
        pop 	es
        pop 	bp
        ret
getppriority_	endp

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
        cmp     ax,22h
        jz      ccpdrv_cp11sv
        cmp     ax,1430h
        jae     ccpdrv_ccp3xsd
        cmp     ax,1041h
        jz      ccpdrv_4xsv
        cmp     ax,1050h
        jz      ccpdrv_4xsv
        jmp     ccpdrv_none
ccpdrv_cp11sv:
        mov     cx,31h
        int     0E0h
        add     bx, 17h
        xor     ah, ah
        mov     al, es:[bx] 
        jmp     ccpdrv_end 
ccpdrv_ccp3xsd:
        mov     cx, 9Ah
        int     0E0h
        xor     ah,ah
        add     bx, 4Bh
        mov     al, es:[bx] 
        jmp     ccpdrv_end
ccpdrv_4xsv:
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
ccpdrv_none:
        mov     ax,0FFFh
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
        mov     si,word ptr 4[bp]
delay_more:
        mov     ax,si
        cmp     ax,21845                 ; 3 * chunk must fit in DX
        jbe     delay_chunk
        mov     ax,21845
delay_chunk:
        sub     si,ax
        mov     bx,3
        mul     bx
        mov     dx,ax
        mov     cx,8Dh
        push    si                       ; BDOS may clobber SI
        int     0E0h
        pop     si
        or      si,si
        jnz     delay_more
        jmp     delay_end
delay_bios:
        mov     ah, 00h
        int     01Ah
        mov     bx, dx                  ; start tick (low word)
delay_lp:
        mov     ah, 00h
        int     01Ah
        mov     ax, dx
        sub     ax, bx                  ; elapsed ticks, wraps modulo 64K
        cmp     ax, word ptr 4[bp]
        jb      delay_lp
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

	        public	ostype_
ostype_ 	proc	near
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
            mov     al,ah
            xor     ah,ah
            cmp     al,0
            jnz     ostypeend
            mov     al,10h
ostypeend:
            pop     cx
            pop     bx
            pop	    si
            pop 	di
            pop     es
            pop     ds
            pop     bp
            ret
ostype_     endp

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

	    public	xstatline_
xstatline_	proc	near
        push	bp
        mov 	bp,sp
        push    es
        push    ds
        push    si
        push    ax
        push    bx
        push    cx
        mov     cl,0Ch
        int     0E0h
        push    ax
        mov     cl,9Ah
        int     0E0h
        mov	    cx, word ptr 4[bp]
        xor     ch,ch
        pop     ax
        cmp     ax, 1041h
        jz      xst_dosplus
        cmp     ax, 1432h
        jb      xst_exit
        push    es
        pop     ds                       ; SYSDAT before reading system pointers
        mov     si, word ptr [68h] ;[4Eh] for Dos Plus
        jmp     xst_call
xst_dosplus:
        push    es
        pop     ds                       ; SYSDAT
        mov     si, word ptr [4Eh] 
xst_call:
        mov     dx,0
        mov     es, 10h[si]              ; UDA segment from system PD
        mov     ax,20H 
        call    dword ptr[28h]
xst_exit:
        pop     cx
        pop     bx
        pop     ax
        pop     si
        pop     ds
        pop     es
        pop     bp
        ret
xstatline_  endp

;
; ccpseg: locate the CP/M-86 1.1 CCP and check it is the layout we expect.
; Returns CF clear and ES = CCP segment, or CF set (do not touch the CCP).
;  - BDOS fn 12 must return 22h (CP/M-86 1.1).
;  - BDOS fn 49 returns ES = CCP/BDOS/BIOS load segment; 0FFh = not
;    implemented (e.g. emu2).
;  - Signature in the CCP data area (labels MENUSIG/SUBNAM in ccp.a86,
;    ccpexp.a86, ccpnew.a86): 'CMD' at 0800h and '$$$' at 0807h (bit 7
;    masked, the BDOS can set attribute bits there). Only when it matches
;    are MDSUBE (0805h) and CMBUFF (0009h) where we think they are.
; Clobbers AX,BX,CX,DX,ES.
;
ccpseg	proc	near
	mov	cx,0Ch			; return version number
	int	0E0h
	cmp	ax,22h
	jnz	ccpseg_no
	mov	cx,31h			; get system data address -> ES = CCP seg
	int	0E0h
	cmp	al,0FFh
	jz	ccpseg_no
	cmp	word ptr es:[0800h],4D43h	; 'C','M'
	jnz	ccpseg_no
	cmp	byte ptr es:[0802h],44h		; 'D'
	jnz	ccpseg_no
	mov	bx,0807h
ccpseg_sub:
	mov	al,es:[bx]
	and	al,7Fh
	cmp	al,24h				; '$'
	jnz	ccpseg_no
	inc	bx
	cmp	bx,080Ah
	jb	ccpseg_sub
	clc
	ret
ccpseg_no:
	stc
	ret
ccpseg	endp

;
; setsub: set the CCP submit-mode flag MDSUBE (CCP offset 0805h) so that
; the warm-boot path (WBOOT -> CCPHOT, which does not re-run BDOS fn 13)
; processes $$$.SUB. Does nothing unless ccpseg recognises the CCP.
; Clobbers AX,BX,CX,DX,ES.
;
setsub	proc	near
	call	ccpseg
	jc	setsub_end
	mov	byte ptr es:[0805h],0FFh ; MDSUBE
setsub_end:
	ret
setsub	endp

;
; sub_active: is the CCP in submit mode (MDSUBE != 0)? True when this
; program was started from a $$$.SUB line; then the records still in
; $$$.SUB are the rest of an outer job and must be kept. When started
; from the keyboard the CCP has already cleared MDSUBE (DELSUB), so any
; $$$.SUB still around is stale.
;
; int sub_active(void)  -- 1 = on, 0 = off, -1 = unknown (ccpseg failed)
;
	public	sub_active_
sub_active_	proc	near
	push	bp
	mov	bp,sp
	push	es
	push	bx
	push	cx
	push	dx
	call	ccpseg
	mov	ax,-1
	jc	active_end
	xor	ax,ax
	cmp	byte ptr es:[0805h],0	; MDSUBE
	jz	active_end
	inc	ax
active_end:
	pop	dx
	pop	cx
	pop	bx
	pop	es
	pop	bp
	ret
sub_active_	endp

;
; sub_cmddrv: drive prefix of the command line the CCP is currently running,
; i.e. the drive the running program was loaded from when typed as "B:MENU".
; The CCP keeps the (upcased, NUL-terminated) line in CMBUFF+2 = offset 000Bh
; of its segment while the transient runs, whether it came from the keyboard,
; $$$.SUB or BDOS fn 47. Same guards as setsub (ccpseg).
;
; int sub_cmddrv(void)  -- 1..16 for A..P, 0 if no prefix or unknown
;
	public	sub_cmddrv_
sub_cmddrv_	proc	near
	push	bp
	mov	bp,sp
	push	es
	push	si
	push	bx
	push	cx
	push	dx
	call	ccpseg
	jc	cmddrv_none
	mov	si,0Bh			; CMBUFF+2
	mov	cx,127
cmddrv_skip:
	mov	al,es:[si]
	cmp	al,' '
	jnz	cmddrv_chk
	inc	si
	loop	cmddrv_skip
	jmp	cmddrv_none
cmddrv_chk:
	cmp	byte ptr es:1[si],':'
	jnz	cmddrv_none
	sub	al,'A'
	cmp	al,15
	ja	cmddrv_none		; not A..P
	inc	al
	xor	ah,ah
	jmp	cmddrv_end
cmddrv_none:
	xor	ax,ax
cmddrv_end:
	pop	dx
	pop	cx
	pop	bx
	pop	si
	pop	es
	pop	bp
	ret
sub_cmddrv_	endp

;
; p_chain: BDOS fn 47 -- chain to program.
; MCHAIN copies a NUL-terminated command line from the current DMA
; address into the CCP command buffer; the CCP hot-start path does not
; upcase it, so the command is upcased here into chainbuf, which is then
; made the DMA buffer. If submode is non-zero, MDSUBE is set so the CCP
; processes $$$.SUB after the chained program exits.
; Does not return on success.
;
; void p_chain(char *cmd, int submode)
;
	public	p_chain_
p_chain_	proc	near
	push	bp
	mov	bp,sp
	push	si
	push	di
	push	es
	cld
	push	ds
	pop	es
	mov	si,word ptr 4[bp]
	mov	di,offset chainbuf
	mov	cx,127
p_chain_copy:
	lodsb
	or	al,al
	jz	p_chain_done
	cmp	al,'a'
	jb	p_chain_store
	cmp	al,'z'
	ja	p_chain_store
	sub	al,20h
p_chain_store:
	stosb
	loop	p_chain_copy
p_chain_done:
	mov	byte ptr [di],0
	mov	dx,ds
	mov	cx,33h			; set DMA segment = DS
	int	0E0h
	mov	dx,offset chainbuf
	mov	cx,1Ah			; set DMA offset = chainbuf
	int	0E0h
	cmp	word ptr 6[bp],0
	jz	p_chain_go
	call	setsub
p_chain_go:
	mov	cx,2Fh			; P_CHAIN = BDOS fn 47
	xor	dx,dx
	int	0E0h			; does not return on success
	pop	es
	pop	di
	pop	si
	pop	bp
	ret
p_chain_	endp

;
; sub_exit: set MDSUBE in the CCP segment (see setsub), then BDOS fn 0
; (system reset) so the CCP picks up $$$.SUB. Mirrors scd.a86 (SUBMIT),
; which writes savess:0805h with savess = CCP segment.
;
	public	sub_exit_
sub_exit_	proc	near
	call	setsub
	xor	cx,cx			; BDOS fn 0 (system reset)
	xor	dx,dx
	int	0E0h
	ret
sub_exit_	endp

codeseg	ends

	    end
