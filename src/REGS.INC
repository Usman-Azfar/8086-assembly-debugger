;==============================================================================
; REGS.INC - the debuggee's register file and the R command
;
; The registers of the program being debugged are kept in memory.  They are
; loaded into the real CPU only while the T command executes an instruction,
; and saved back from the real CPU right after it (see TRACE.INC).
;
; Everything here is table driven: reg_names and regs are in the same order,
; so register N's name is at reg_names+2*N and its value at regs+2*N.
;==============================================================================

NUM_REGS	equ 13
TRAP_FLAG	equ 0100h

	.data
reg_names	db "AXBXCXDXSPBPSIDIDSESSSCSIP"

regs		label word
r_ax		dw 0
r_bx		dw 0
r_cx		dw 0
r_dx		dw 0
r_sp		dw 0FFFEh
r_bp		dw 0
r_si		dw 0
r_di		dw 0
r_ds		dw 0			; the four segment registers are set to the
r_es		dw 0			; sandbox segment at start-up
r_ss		dw 0
r_cs		dw 0
r_ip		dw 0100h
r_fl		dw 0202h		; interrupts enabled, everything else clear

; One entry per flag DEBUG.COM shows: bit mask, text when set, text when clear
flag_tab	dw 0800h
		db "OVNV"		; overflow
		dw 0400h
		db "DNUP"		; direction
		dw 0200h
		db "EIDI"		; interrupt
		dw 0080h
		db "NGPL"		; sign
		dw 0040h
		db "ZRNZ"		; zero
		dw 0010h
		db "ACNA"		; auxiliary carry
		dw 0004h
		db "PEPO"		; parity
		dw 0001h
		db "CYNC"		; carry
NUM_FLAGS	equ 8
FLAG_ENTRY	equ 6

; Segment registers that may prefix an address, e.g. "D CS:100"
seg_names	db "CS"
		dw offset r_cs
		db "DS"
		dw offset r_ds
		db "ES"
		dw offset r_es
		db "SS"
		dw offset r_ss

flags_prompt	db " -$"

	.code

;------------------------------------------------------------------------------
; show_regs - print all registers, the flags and the next instruction,
; in the same layout as DEBUG.COM's R command
;------------------------------------------------------------------------------
show_regs proc
	push ax
	push bx
	push cx
	push dx
	xor bx,bx			; BX = 2 * register number
sr_loop:
	or bx,bx
	jz sr_name			; first register: no separator
	cmp bx,2*8
	jne sr_gap
	call put_crlf			; DS starts the second line
	jmp sr_name
sr_gap:
	mov cx,2
	call put_spaces
sr_name:
	mov dl,reg_names[bx]
	call put_char
	mov dl,reg_names[bx+1]
	call put_char
	mov dl,'='
	call put_char
	mov ax,regs[bx]
	call put_hex16
	add bx,2
	cmp bx,2*NUM_REGS
	jb sr_loop

	mov cx,3
	call put_spaces
	call show_flags
	call put_crlf

	mov bx,r_cs			; the instruction at CS:IP
	mov ax,r_ip
	call disasm_line
	pop dx
	pop cx
	pop bx
	pop ax
	ret
show_regs endp

;------------------------------------------------------------------------------
; show_flags - print the flags as "NV UP EI PL NZ NA PO NC"
;------------------------------------------------------------------------------
show_flags proc
	push ax
	push cx
	push dx
	push si
	mov si,offset flag_tab
	mov cx,NUM_FLAGS
sf_loop:
	mov ax,[si]
	test r_fl,ax
	jnz sf_set
	mov dl,[si+4]			; flag clear
	call put_char
	mov dl,[si+5]
	jmp sf_print
sf_set:
	mov dl,[si+2]			; flag set
	call put_char
	mov dl,[si+3]
sf_print:
	call put_char
	add si,FLAG_ENTRY
	cmp cx,1
	je sf_next
	mov dl,' '
	call put_char
sf_next:
	loop sf_loop
	pop si
	pop dx
	pop cx
	pop ax
	ret
show_flags endp

;------------------------------------------------------------------------------
; cmd_r - R            show registers
;         R reg        change a register (AX BX CX DX SP BP SI DI DS ES SS CS IP)
;         R F          change flags
;------------------------------------------------------------------------------
cmd_r proc
	call skip_blanks
	or al,al
	jnz cr_named
	call show_regs
	ret

cr_named:
	mov err_pos,si
	cmp al,'F'			; "R F" ?
	jne cr_reg
	mov al,[si+1]
	or al,al
	jz cr_flags
	cmp al,' '
	jne cr_reg
cr_flags:
	inc si
	call expect_end
	jc cr_bad
	call edit_flags
	ret

cr_reg:
	mov ax,[si]			; two-letter register name
	mov di,offset reg_names
	mov cx,NUM_REGS
	xor bx,bx
cr_find:
	cmp ax,[di]
	je cr_found
	add di,2
	inc bx
	loop cr_find
	jmp cr_bad
cr_found:
	add si,2
	call expect_end
	jc cr_bad
	call edit_register
	ret
cr_bad:
	call show_error
	ret
cmd_r endp

;------------------------------------------------------------------------------
; edit_register - show register BX and read a new value for it.
; Pressing Enter on an empty line keeps the old value.
;------------------------------------------------------------------------------
edit_register proc
	shl bx,1
	mov dl,reg_names[bx]
	call put_char
	mov dl,reg_names[bx+1]
	call put_char
	mov dl,' '
	call put_char
	mov ax,regs[bx]
	call put_hex16
	call put_crlf
	mov dl,':'
	call put_char

	mov prompt_len,1
	call read_line
	call skip_blanks
	or al,al
	jz er_done			; empty line: keep the value
	call parse_hex
	jc er_bad
	push ax
	call expect_end
	pop ax
	jc er_bad
	mov regs[bx],ax
er_done:
	ret
er_bad:
	call show_error
	ret
edit_register endp

;------------------------------------------------------------------------------
; edit_flags - show the flags and read a list of new flag states, e.g.
; "CY ZR" sets carry and zero.  Nothing changes unless the whole list is valid.
;------------------------------------------------------------------------------
edit_flags proc
	call show_flags
	mov dx,offset flags_prompt
	call put_str

	mov prompt_len,8*3+1		; "NV UP EI PL NZ NA PO NC -"
	call read_line
	mov dx,r_fl			; DX = working copy of the flags
ef_loop:
	call skip_blanks
	or al,al
	jz ef_commit
	mov err_pos,si
	mov ax,[si]
	mov di,offset flag_tab
	mov cx,NUM_FLAGS
ef_find:
	cmp ax,[di+2]
	je ef_set
	cmp ax,[di+4]
	je ef_clear
	add di,FLAG_ENTRY
	loop ef_find
	call show_error			; not a flag name
	ret
ef_set:
	or dx,[di]
	jmp ef_next
ef_clear:
	mov bx,[di]
	not bx
	and dx,bx
ef_next:
	add si,2
	jmp ef_loop
ef_commit:
	mov r_fl,dx
	ret
edit_flags endp
