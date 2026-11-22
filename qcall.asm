; Aztec near-call bridge for the native BDOS word result and error code.
codeseg segment para public 'code'
        assume cs:codeseg
        public qcall_
qcall_  proc near
        push bp
        mov  bp,sp
        push bx
        push si
        push di
        push es
        mov  cx,4[bp]
        mov  dx,6[bp]
        int  0e0h
        cmp  word ptr 4[bp],12
        je   qdone
        mov  ax,bx
        mov  bx,8[bp]
        mov  [bx],cx
qdone:  pop  es
        pop  di
        pop  si
        pop  bx
        pop  bp
        ret
qcall_  endp
codeseg ends
        end
