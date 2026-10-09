;==============================================================================
; DISASM.INC - 8086 disassembler and the U command
;
; Decodes the complete 8086 instruction set (the 186+ opcodes 60h-6Fh,
; C0h, C1h, C8h, C9h and the undocumented ones are shown as "DB xx").
; Prefixes (segment overrides, REP, LOCK) are shown as separate one-byte
; instructions, the same way DEBUG.COM shows them.
;
; The text of an instruction is built with the normal print routines while
; output is captured into txtbuf (see put_char), so the byte column can be
; printed before it once the instruction length is known.
;==============================================================================

	.data
u_seg		dw 0			; where a plain "U" continues from
u_off		dw 0100h

dis_seg		dw 0			; instruction being decoded
dis_off		dw 0			; next byte to fetch
dis_start	dw 0			; first byte of the instruction
hexcnt		db 0			; bytes fetched so far
hexbytes	db 8 dup(0)
txtbuf		db 64 dup(0)

d_op		db 0			; opcode byte
d_mod		db 0			; ModR/M fields
d_reg		db 0
d_rm		db 0
d_w		db 0			; operand size: 0 = byte, 1 = word
d_ptr		db 0			; 1 = say BYTE PTR / WORD PTR for memory

; Name tables: 6 bytes per entry, padded with zeros
alu_names	db "ADD",0,0,0, "OR",0,0,0,0, "ADC",0,0,0, "SBB",0,0,0
		db "AND",0,0,0, "SUB",0,0,0,  "XOR",0,0,0, "CMP",0,0,0
stack_names	db "INC",0,0,0, "DEC",0,0,0,  "PUSH",0,0,  "POP",0,0,0
jcc_names	db "JO",0,0,0,0, "JNO",0,0,0, "JB",0,0,0,0, "JNB",0,0,0
		db "JZ",0,0,0,0, "JNZ",0,0,0, "JBE",0,0,0,  "JA",0,0,0,0
		db "JS",0,0,0,0, "JNS",0,0,0, "JPE",0,0,0,  "JPO",0,0,0
		db "JL",0,0,0,0, "JGE",0,0,0, "JLE",0,0,0,  "JG",0,0,0,0
shift_names	db "ROL",0,0,0, "ROR",0,0,0,  "RCL",0,0,0, "RCR",0,0,0
		db "SHL",0,0,0, "SHR",0,0,0,  "???",0,0,0, "SAR",0,0,0
grp3_names	db "TEST",0,0,  "TEST",0,0,   "NOT",0,0,0, "NEG",0,0,0
		db "MUL",0,0,0, "IMUL",0,0,   "DIV",0,0,0, "IDIV",0,0
grp5_names	db "INC",0,0,0, "DEC",0,0,0,  "CALL",0,0,  "CALL",0,0
		db "JMP",0,0,0, "JMP",0,0,0,  "PUSH",0,0,  "???",0,0,0
loop_names	db "LOOPNZ",    "LOOPZ",0,    "LOOP",0,0,  "JCXZ",0,0
rm_names	db "BX+SI",0,   "BX+DI",0,    "BP+SI",0,   "BP+DI",0
		db "SI",0,0,0,0,"DI",0,0,0,0, "BP",0,0,0,0,"BX",0,0,0,0

reg8_names	db "ALCLDLBLAHCHDHBH"
reg16_names	db "AXCXDXBXSPBPSIDI"
sreg_names	db "ESCSSSDS"

; Instructions with no operands: opcode followed by a 6-byte name
simple_ops	db 27h,"DAA",0,0,0,   2Fh,"DAS",0,0,0,   37h,"AAA",0,0,0
		db 3Fh,"AAS",0,0,0,   90h,"NOP",0,0,0,   98h,"CBW",0,0,0
		db 99h,"CWD",0,0,0,   9Bh,"WAIT",0,0,    9Ch,"PUSHF",0
		db 9Dh,"POPF",0,0,    9Eh,"SAHF",0,0,    9Fh,"LAHF",0,0
		db 0A4h,"MOVSB",0,    0A5h,"MOVSW",0,    0A6h,"CMPSB",0
		db 0A7h,"CMPSW",0,    0AAh,"STOSB",0,    0ABh,"STOSW",0
		db 0ACh,"LODSB",0,    0ADh,"LODSW",0,    0AEh,"SCASB",0
		db 0AFh,"SCASW",0,    0C3h,"RET",0,0,0,  0CBh,"RETF",0,0
		db 0CEh,"INTO",0,0,   0CFh,"IRET",0,0,   0D7h,"XLAT",0,0
		db 0F0h,"LOCK",0,0,   0F2h,"REPNZ",0,    0F3h,"REPZ",0,0
		db 0F4h,"HLT",0,0,0,  0F5h,"CMC",0,0,0,  0F8h,"CLC",0,0,0
		db 0F9h,"STC",0,0,0,  0FAh,"CLI",0,0,0,  0FBh,"STI",0,0,0
		db 0FCh,"CLD",0,0,0,  0FDh,"STD",0,0,0
SIMPLE_COUNT	equ ($ - simple_ops) / 7

s_mov		db "MOV",0
s_test		db "TEST",0
s_xchg		db "XCHG",0
s_lea		db "LEA",0
s_les		db "LES",0
s_lds		db "LDS",0
s_pop		db "POP",0
s_int		db "INT",0
s_ret		db "RET",0
s_retf		db "RETF",0
s_call		db "CALL",0
s_jmp		db "JMP",0
s_in		db "IN",0
s_out		db "OUT",0
s_aam		db "AAM",0
s_aad		db "AAD",0
s_esc		db "ESC",0
s_db		db "DB",0
s_dx		db "DX",0
s_cl		db "CL",0
s_far		db "FAR ",0
s_byte_ptr	db "BYTE PTR ",0
s_word_ptr	db "WORD PTR ",0

	.code

;------------------------------------------------------------------------------
; disasm_line - disassemble and print the instruction at BX:AX as
;   SSSS:OOOO BYTES         MNEMONIC OPERANDS
; Returns AX = offset of the next instruction.  Other registers preserved.
;------------------------------------------------------------------------------
disasm_line proc
	push bx
	push cx
	push dx
	push si
	push di
	mov dis_seg,bx
	mov dis_off,ax
	mov dis_start,ax
	mov hexcnt,0

	mov cap_ptr,offset txtbuf	; build the text in txtbuf
	call dis_decode
	mov di,cap_ptr
	mov byte ptr [di],'$'
	mov cap_ptr,0			; back to the screen

	mov ax,dis_seg
	call put_hex16
	mov dl,':'
	call put_char
	mov ax,dis_start
	call put_hex16
	mov dl,' '
	call put_char

	xor bx,bx
dl_bytes:
	cmp bl,hexcnt
	jae dl_pad
	mov al,hexbytes[bx]
	call put_hex8
	inc bx
	jmp dl_bytes
dl_pad:
	mov cx,14			; byte column is 14 characters wide
	mov al,hexcnt
	xor ah,ah
	shl ax,1
	cmp ax,cx
	jb dl_pad_ok
	mov ax,13
dl_pad_ok:
	sub cx,ax
	call put_spaces
	mov dx,offset txtbuf
	call put_str
	call put_crlf

	mov ax,dis_off
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	ret
disasm_line endp

;------------------------------------------------------------------------------
; cmd_u - U [address] [range]     (default 20h bytes, continues where the
;                                  previous U stopped)
;------------------------------------------------------------------------------
cmd_u proc
	call skip_blanks
	mov bx,u_seg
	mov dx,u_off
	or al,al
	jz cu_range
	mov bx,r_cs
	call parse_address
	jc cu_bad
	mov dx,ax
cu_range:
	mov ax,dx
	mov cx,20h
	call parse_range
	jc cu_bad
	call expect_end
	jc cu_bad
	mov dx,cx			; DX = last offset to start an instruction at
cu_loop:
	mov si,ax
	call disasm_line
	cmp ax,si
	jbe cu_stop			; wrapped past the end of the segment
	cmp ax,dx
	jbe cu_loop
cu_stop:
	mov u_seg,bx
	mov u_off,ax
	ret
cu_bad:
	call show_error
	ret
cmd_u endp

;==============================================================================
; Small helpers used by the decoder.  All preserve registers except outputs.
;==============================================================================

; dfetch - AL = next instruction byte
dfetch proc
	push bx
	push es
	mov es,dis_seg
	mov bx,dis_off
	mov al,es:[bx]
	inc dis_off
	mov bl,hexcnt
	cmp bl,8
	jae df_done
	xor bh,bh
	mov hexbytes[bx],al
	inc hexcnt
df_done:
	pop es
	pop bx
	ret
dfetch endp

; dfetchw - AX = next instruction word
dfetchw proc
	call dfetch
	mov ah,al
	call dfetch
	xchg al,ah
	ret
dfetchw endp

; dname - print the name at SI (at most 6 characters, stops at a 0 byte)
dname proc
	push cx
	push dx
	push si
	mov cx,6
dn_loop:
	mov dl,[si]
	or dl,dl
	jz dn_done
	call put_char
	inc si
	loop dn_loop
dn_done:
	pop si
	pop dx
	pop cx
	ret
dname endp

; dstr - print the 0-terminated string at SI
dstr proc
	push dx
	push si
ds_loop:
	mov dl,[si]
	or dl,dl
	jz ds_done
	call put_char
	inc si
	jmp ds_loop
ds_done:
	pop si
	pop dx
	ret
dstr endp

; dmnem - print the mnemonic at SI and pad it so operands line up
dmnem proc
	push ax
	push dx
	call dname
dm_pad:
	mov ax,cap_ptr
	sub ax,offset txtbuf
	cmp ax,8
	jae dm_pad_done
	mov dl,' '
	call put_char
	jmp dm_pad
dm_pad_done:
	pop dx
	pop ax
	ret
dmnem endp

; dmnem_idx - print entry AL of the 6-byte name table at SI as a mnemonic
dmnem_idx proc
	push ax
	push si
	xor ah,ah
	shl ax,1
	add si,ax			; SI += 2 * AL
	shl ax,1
	add si,ax			; SI += 4 * AL, so 6 * AL in total
	call dmnem
	pop si
	pop ax
	ret
dmnem_idx endp

; dreg - print general register AL (0..7), byte or word according to d_w
dreg proc
	push ax
	push dx
	push si
	and ax,7
	shl ax,1
	mov si,offset reg8_names
	cmp d_w,0
	je dr_print
	mov si,offset reg16_names
dr_print:
	add si,ax
	mov dl,[si]
	call put_char
	mov dl,[si+1]
	call put_char
	pop si
	pop dx
	pop ax
	ret
dreg endp

; dsreg - print segment register AL (0..3 = ES CS SS DS)
dsreg proc
	push ax
	push dx
	push si
	and ax,3
	shl ax,1
	mov si,offset sreg_names
	add si,ax
	mov dl,[si]
	call put_char
	mov dl,[si+1]
	call put_char
	pop si
	pop dx
	pop ax
	ret
dsreg endp

; dcomma - print ","
dcomma proc
	push dx
	mov dl,','
	call put_char
	pop dx
	ret
dcomma endp

; dimm - fetch and print an immediate operand of size d_w
dimm proc
	push ax
	cmp d_w,0
	je di_byte
	call dfetchw
	call put_hex16
	pop ax
	ret
di_byte:
	call dfetch
	call put_hex8
	pop ax
	ret
dimm endp

; dsigned8 - print AL as a signed displacement: +12 or -12
dsigned8 proc
	push ax
	push dx
	mov dl,'+'
	test al,80h
	jz dsg_print
	neg al
	mov dl,'-'
dsg_print:
	call put_char
	call put_hex8
	pop dx
	pop ax
	ret
dsigned8 endp

; drel8 / drel16 - fetch a relative displacement and print the target address
drel8 proc
	push ax
	call dfetch
	cbw
	add ax,dis_off
	call put_hex16
	pop ax
	ret
drel8 endp

drel16 proc
	push ax
	call dfetchw
	add ax,dis_off
	call put_hex16
	pop ax
	ret
drel16 endp

; dfarptr - fetch and print a far address as SSSS:OOOO
dfarptr proc
	push ax
	push bx
	call dfetchw
	mov bx,ax			; offset comes first in the instruction
	call dfetchw
	call put_hex16
	push dx
	mov dl,':'
	call put_char
	pop dx
	mov ax,bx
	call put_hex16
	pop bx
	pop ax
	ret
dfarptr endp

; dmodrm - fetch a ModR/M byte and split it into d_mod, d_reg and d_rm
dmodrm proc
	push ax
	push cx
	call dfetch
	mov ah,al
	and al,7
	mov d_rm,al
	mov al,ah
	mov cl,3
	shr al,cl
	and al,7
	mov d_reg,al
	mov al,ah
	mov cl,6
	shr al,cl
	mov d_mod,al
	pop cx
	pop ax
	ret
dmodrm endp

; drm - print the register or memory operand described by the ModR/M byte
drm proc
	push ax
	push dx
	push si
	cmp d_mod,3
	jne drm_memory
	mov al,d_rm
	call dreg
	jmp drm_done

drm_memory:
	cmp d_ptr,0
	je drm_open
	mov si,offset s_byte_ptr
	cmp d_w,0
	je drm_size
	mov si,offset s_word_ptr
drm_size:
	call dstr
drm_open:
	mov dl,'['
	call put_char
	cmp d_mod,0
	jne drm_base
	cmp d_rm,6
	jne drm_base
	call dfetchw			; [nnnn] direct address
	call put_hex16
	jmp drm_close
drm_base:
	mov al,d_rm
	xor ah,ah
	mov si,offset rm_names
	shl ax,1
	add si,ax
	shl ax,1
	add si,ax			; SI = rm_names + 6 * rm
	call dname
	cmp d_mod,1
	je drm_disp8
	cmp d_mod,2
	jne drm_close
	mov dl,'+'			; 16-bit displacement
	call put_char
	call dfetchw
	call put_hex16
	jmp drm_close
drm_disp8:
	call dfetch
	call dsigned8
drm_close:
	mov dl,']'
	call put_char
drm_done:
	pop si
	pop dx
	pop ax
	ret
drm endp

; drm_reg - print "r/m,reg" (direction bit clear) or "reg,r/m" (set)
drm_reg proc
	push ax
	test d_op,2
	jnz drr_reg_first
	call drm
	call dcomma
	mov al,d_reg
	call dreg
	pop ax
	ret
drr_reg_first:
	mov al,d_reg
	call dreg
	call dcomma
	call drm
	pop ax
	ret
drm_reg endp

;==============================================================================
; dis_decode - decode one instruction at dis_seg:dis_off into the output
;==============================================================================
dis_decode proc
	mov d_ptr,0
	mov d_w,0
	call dfetch
	mov d_op,al

	mov si,offset simple_ops	; instructions without operands
	mov cx,SIMPLE_COUNT
dd_scan:
	cmp al,[si]
	je dd_simple
	add si,7
	loop dd_scan

	cmp al,40h
	jb dd_alu
	cmp al,60h
	jb dd_incdec
	cmp al,70h
	jb dd_bad
	cmp al,80h
	jb dd_jcc
	cmp al,84h
	jb dd_grp1
	cmp al,88h
	jb dd_test_xchg
	cmp al,8Ch
	jb dd_mov
	cmp al,90h
	jb dd_8c_8f
	cmp al,98h
	jb dd_xchg_ax
	cmp al,9Ah
	je dd_call_far
	cmp al,0A0h
	jb dd_bad
	cmp al,0A4h
	jb dd_mov_mem
	cmp al,0A8h
	je dd_test_acc
	cmp al,0A9h
	je dd_test_acc
	cmp al,0B0h
	jb dd_bad
	cmp al,0C0h
	jb dd_mov_imm
	cmp al,0D0h
	jb dd_cx
	cmp al,0D8h
	jb dd_dx
	cmp al,0E0h
	jb dd_esc
	cmp al,0F0h
	jb dd_ex
	jmp dd_fx

dd_simple:
	inc si
	call dname
	ret

dd_bad:					; not an 8086 instruction
	mov si,offset s_db
	call dmnem
	mov al,d_op
	call put_hex8
	ret

;--- 00h-3Fh: ADD OR ADC SBB AND SUB XOR CMP, PUSH/POP sreg, prefixes ---------
dd_alu:
	mov bl,al
	and bl,7			; BL = low 3 bits
	mov bh,al
	mov cl,3
	shr bh,cl			; BH = bits 5..3
	cmp bl,6
	jae dd_alu_67
	mov al,bh
	mov si,offset alu_names
	call dmnem_idx
	mov al,d_op
	and al,1
	mov d_w,al
	cmp bl,4
	jae dd_alu_acc
	call dmodrm
	call drm_reg
	ret
dd_alu_acc:				; AL,imm8 / AX,imm16
	xor al,al
	call dreg
	call dcomma
	call dimm
	ret
dd_alu_67:
	cmp bh,4
	jae dd_prefix
	cmp d_op,0Fh			; POP CS is not a usable instruction
	je dd_bad
	mov al,2			; PUSH
	cmp bl,6
	je dd_alu_stack
	mov al,3			; POP
dd_alu_stack:
	mov si,offset stack_names
	call dmnem_idx
	mov al,bh
	call dsreg
	ret
dd_prefix:				; 26h 2Eh 36h 3Eh = ES: CS: SS: DS:
	mov al,bh
	sub al,4
	call dsreg
	mov dl,':'
	call put_char
	ret

;--- 40h-5Fh: INC DEC PUSH POP r16 -------------------------------------------
dd_incdec:
	mov bl,al
	mov cl,3
	shr al,cl
	and al,3
	mov si,offset stack_names
	call dmnem_idx
	mov d_w,1
	mov al,bl
	call dreg
	ret

;--- 70h-7Fh: conditional jumps ----------------------------------------------
dd_jcc:
	and al,0Fh
	mov si,offset jcc_names
	call dmnem_idx
	call drel8
	ret

;--- 80h-83h: ALU r/m,imm ----------------------------------------------------
dd_grp1:
	and al,1
	mov d_w,al
	call dmodrm
	mov al,d_reg
	mov si,offset alu_names
	call dmnem_idx
	mov d_ptr,1
	call drm
	call dcomma
	cmp d_op,83h
	je dd_grp1_sext
	call dimm
	ret
dd_grp1_sext:				; byte sign-extended to a word
	call dfetch
	call dsigned8
	ret

;--- 84h-87h: TEST / XCHG r/m,reg --------------------------------------------
dd_test_xchg:
	mov si,offset s_test
	cmp al,86h
	jb dd_tx_name
	mov si,offset s_xchg
dd_tx_name:
	and al,1
	mov d_w,al
	call dmodrm
	call dmnem
	call drm
	call dcomma
	mov al,d_reg
	call dreg
	ret

;--- 88h-8Bh: MOV r/m,reg / reg,r/m ------------------------------------------
dd_mov:
	and al,1
	mov d_w,al
	call dmodrm
	mov si,offset s_mov
	call dmnem
	call drm_reg
	ret

;--- 8Ch-8Fh: MOV r/m,sreg  LEA  MOV sreg,r/m  POP r/m ------------------------
dd_8c_8f:
	mov d_w,1
	call dmodrm
	cmp al,8Dh
	je dd_lea
	cmp al,8Fh
	je dd_pop_rm
	mov si,offset s_mov
	call dmnem
	cmp al,8Eh
	je dd_mov_to_sreg
	call drm			; 8Ch
	call dcomma
	mov al,d_reg
	call dsreg
	ret
dd_mov_to_sreg:
	mov al,d_reg
	call dsreg
	call dcomma
	call drm
	ret
dd_lea:
	mov si,offset s_lea
	call dmnem
	mov al,d_reg
	call dreg
	call dcomma
	call drm
	ret
dd_pop_rm:
	mov si,offset s_pop
	call dmnem
	mov d_ptr,1
	call drm
	ret

;--- 91h-97h: XCHG AX,r16 ----------------------------------------------------
dd_xchg_ax:
	mov d_w,1
	mov si,offset s_xchg
	call dmnem
	xor al,al
	call dreg
	call dcomma
	mov al,d_op
	call dreg
	ret

;--- 9Ah: CALL far -----------------------------------------------------------
dd_call_far:
	mov si,offset s_call
	call dmnem
	call dfarptr
	ret

;--- A0h-A3h: MOV accumulator <-> [address] ----------------------------------
dd_mov_mem:
	and al,1
	mov d_w,al
	mov si,offset s_mov
	call dmnem
	test d_op,2
	jnz dd_mm_store
	xor al,al			; MOV AL/AX,[nnnn]
	call dreg
	call dcomma
	mov dl,'['
	call put_char
	call dfetchw
	call put_hex16
	mov dl,']'
	call put_char
	ret
dd_mm_store:				; MOV [nnnn],AL/AX
	mov dl,'['
	call put_char
	call dfetchw
	call put_hex16
	mov dl,']'
	call put_char
	call dcomma
	xor al,al
	call dreg
	ret

;--- A8h-A9h: TEST accumulator,imm -------------------------------------------
dd_test_acc:
	and al,1
	mov d_w,al
	mov si,offset s_test
	call dmnem
	xor al,al
	call dreg
	call dcomma
	call dimm
	ret

;--- B0h-BFh: MOV reg,imm ----------------------------------------------------
dd_mov_imm:
	mov bl,al
	mov cl,3
	shr al,cl
	and al,1
	mov d_w,al
	mov si,offset s_mov
	call dmnem
	mov al,bl
	call dreg
	call dcomma
	call dimm
	ret

;--- C0h-CFh: RET imm, LES, LDS, MOV r/m,imm, INT ---------------------------
dd_cx:
	cmp al,0C2h
	je dd_ret_imm
	cmp al,0CAh
	je dd_retf_imm
	cmp al,0C4h
	je dd_les_lds
	cmp al,0C5h
	je dd_les_lds
	cmp al,0C6h
	je dd_mov_rm_imm
	cmp al,0C7h
	je dd_mov_rm_imm
	cmp al,0CCh
	je dd_int3
	cmp al,0CDh
	je dd_int
	jmp dd_bad			; C0h C1h C8h C9h are 186+ only
dd_ret_imm:
	mov si,offset s_ret
	jmp dd_ret_common
dd_retf_imm:
	mov si,offset s_retf
dd_ret_common:
	call dmnem
	call dfetchw
	call put_hex16
	ret
dd_les_lds:
	mov si,offset s_les
	cmp al,0C4h
	je dd_ll_name
	mov si,offset s_lds
dd_ll_name:
	mov d_w,1
	call dmodrm
	call dmnem
	mov al,d_reg
	call dreg
	call dcomma
	call drm
	ret
dd_mov_rm_imm:
	and al,1
	mov d_w,al
	call dmodrm
	mov si,offset s_mov
	call dmnem
	mov d_ptr,1
	call drm
	call dcomma
	call dimm
	ret
dd_int3:
	mov si,offset s_int
	call dmnem
	mov dl,'3'
	call put_char
	ret
dd_int:
	mov si,offset s_int
	call dmnem
	call dfetch
	call put_hex8
	ret

;--- D0h-D7h: shifts and rotates, AAM, AAD -----------------------------------
dd_dx:
	cmp al,0D4h
	je dd_aam
	cmp al,0D5h
	je dd_aad
	cmp al,0D6h
	je dd_bad
	and al,1			; D0h-D3h
	mov d_w,al
	call dmodrm
	mov al,d_reg
	mov si,offset shift_names
	call dmnem_idx
	mov d_ptr,1
	call drm
	call dcomma
	test d_op,2
	jnz dd_shift_cl
	mov dl,'1'
	call put_char
	ret
dd_shift_cl:
	mov si,offset s_cl
	call dstr
	ret
dd_aam:
	mov si,offset s_aam
	jmp dd_aa_common
dd_aad:
	mov si,offset s_aad
dd_aa_common:
	call dname
	call dfetch			; base, normally 0Ah
	cmp al,0Ah
	je dd_aa_done
	push dx
	mov dl,' '
	call put_char
	pop dx
	call put_hex8
dd_aa_done:
	ret

;--- D8h-DFh: ESC (coprocessor) ----------------------------------------------
dd_esc:
	call dmodrm
	mov si,offset s_esc
	call dmnem
	mov al,d_op
	and al,7
	mov cl,3
	shl al,cl
	or al,d_reg
	call put_hex8
	call dcomma
	call drm
	ret

;--- E0h-EFh: LOOPs, IN/OUT, CALL, JMP ---------------------------------------
dd_ex:
	cmp al,0E4h
	jb dd_loop
	cmp al,0E8h
	jb dd_io_imm
	je dd_call_near
	cmp al,0E9h
	je dd_jmp_near
	cmp al,0EAh
	je dd_jmp_far
	cmp al,0EBh
	je dd_jmp_short
	jmp dd_io_dx			; ECh-EFh
dd_loop:
	and al,3
	mov si,offset loop_names
	call dmnem_idx
	call drel8
	ret
dd_io_imm:
	and al,1
	mov d_w,al
	test d_op,2
	jnz dd_out_imm
	mov si,offset s_in		; IN acc,port
	call dmnem
	xor al,al
	call dreg
	call dcomma
	call dfetch
	call put_hex8
	ret
dd_out_imm:
	mov si,offset s_out		; OUT port,acc
	call dmnem
	call dfetch
	call put_hex8
	call dcomma
	xor al,al
	call dreg
	ret
dd_call_near:
	mov si,offset s_call
	call dmnem
	call drel16
	ret
dd_jmp_near:
	mov si,offset s_jmp
	call dmnem
	call drel16
	ret
dd_jmp_far:
	mov si,offset s_jmp
	call dmnem
	call dfarptr
	ret
dd_jmp_short:
	mov si,offset s_jmp
	call dmnem
	call drel8
	ret
dd_io_dx:
	and al,1
	mov d_w,al
	test d_op,2
	jnz dd_out_dx
	mov si,offset s_in		; IN acc,DX
	call dmnem
	xor al,al
	call dreg
	call dcomma
	mov si,offset s_dx
	call dstr
	ret
dd_out_dx:
	mov si,offset s_out		; OUT DX,acc
	call dmnem
	mov si,offset s_dx
	call dstr
	call dcomma
	xor al,al
	call dreg
	ret

;--- F0h-FFh: group 3 (TEST NOT NEG MUL ...), group 4/5 (INC DEC CALL ...) ---
dd_fx:
	cmp al,0F6h
	je dd_grp3
	cmp al,0F7h
	je dd_grp3
	cmp al,0FEh
	je dd_grp4
	cmp al,0FFh
	je dd_grp5
	jmp dd_bad			; F1h
dd_grp3:
	and al,1
	mov d_w,al
	call dmodrm
	mov al,d_reg
	mov si,offset grp3_names
	call dmnem_idx
	mov d_ptr,1
	call drm
	cmp d_reg,2
	jae dd_grp3_done
	call dcomma			; TEST r/m,imm
	call dimm
dd_grp3_done:
	ret
dd_grp4:				; INC/DEC byte r/m
	call dmodrm
	cmp d_reg,2
	jae dd_grp4_bad
	mov al,d_reg
	mov si,offset stack_names
	call dmnem_idx
	mov d_ptr,1
	call drm
	ret
dd_grp4_bad:
	jmp dd_bad
dd_grp5:
	mov d_w,1
	call dmodrm
	mov al,d_reg
	mov si,offset grp5_names
	call dmnem_idx
	cmp al,3			; CALL FAR / JMP FAR
	je dd_grp5_far
	cmp al,5
	je dd_grp5_far
	cmp al,2			; near CALL / JMP: no size needed
	je dd_grp5_rm
	cmp al,4
	je dd_grp5_rm
	mov d_ptr,1			; INC DEC PUSH
	jmp dd_grp5_rm
dd_grp5_far:
	mov si,offset s_far
	call dstr
dd_grp5_rm:
	call drm
	ret
dis_decode endp
