;==============================================================================
; CPROJ.ASM - 8086 Register Command Debugger
;
; A DEBUG.COM-style machine-level monitor for MS-DOS, written in pure 8086
; assembly language.  It gives a program its own 64 KB "sandbox" segment and
; lets you inspect and change its registers, flags and memory, disassemble
; its code and execute it one real instruction at a time.
;
;   R [reg | F]           show / change registers and flags
;   D [addr] [range]      dump memory
;   E addr list           enter bytes into memory
;   F addr range list     fill memory
;   U [addr] [range]      disassemble
;   T [count]             trace (single-step) instructions
;   H val1 val2           hex add / subtract
;   ?                     help
;   Q                     quit
;
; Source layout:
;   CPROJ.ASM     start-up, command loop, help        (this file)
;   CONSOLE.INC   console input/output
;   PARSE.INC     number / address / range / list parsing
;   REGS.INC      register file, R command
;   MEMORY.INC    D, E, F, H commands
;   DISASM.INC    8086 disassembler, U command
;   TRACE.INC     T command (Trap Flag + INT 1 handler)
;
; Build:  jwasm -mz CPROJ.ASM          (JWasm, produces CPROJ.EXE directly)
;    or:  ml CPROJ.ASM                 (MASM 6.x with a 16-bit linker)
;==============================================================================

	.model small
	.8086
	.stack 400h

SANDBOX_PARAS	equ 1000h		; 64 KB, in 16-byte paragraphs

	.data
banner		db 13, 10
		db "8086 Register Command Debugger", 13, 10
		db "Type ? for help.  A demo program is loaded at CS:0100 -", 13, 10
		db "try U to see it, then T to execute it one instruction at a time.", 13, 10, "$"

help_text	db "Commands (letters can be upper or lower case, numbers are hex):", 13, 10
		db "  R                  show registers, flags and the next instruction", 13, 10
		db "  R reg              change a register: AX BX CX DX SP BP SI DI", 13, 10
		db "                                        DS ES SS CS IP", 13, 10
		db "  R F                change flags, e.g. CY ZR (set) or NC NZ (clear)", 13, 10
		db "  D [addr] [range]   dump memory as hex and ASCII", 13, 10
		db "  E addr list        enter bytes, e.g. E 200 41 42 ", 22h, "text", 22h, 13, 10
		db "  F addr range list  fill memory with a repeating pattern", 13, 10
		db "  U [addr] [range]   unassemble (disassemble) instructions", 13, 10
		db "  T [count]          trace: execute instructions one at a time", 13, 10
		db "  H val1 val2        show val1+val2 and val1-val2", 13, 10
		db "  Q                  quit", 13, 10
		db "addr  = offset, seg:offset, or CS: DS: ES: SS: followed by an offset", 13, 10
		db "range = end offset, or L followed by a length", 13, 10, "$"

msg_nomem	db "Not enough memory for the 64 KB sandbox segment.", 13, 10, "$"

; Demo program copied to sandbox:0100.  It is chosen to show the flags
; changing: overflow, zero, sign, auxiliary carry, parity and carry.
demo_code	db 0B8h, 0FFh, 07Fh		; 0100  MOV  AX,7FFF
		db 005h, 001h, 000h		; 0103  ADD  AX,0001   -> OV NG
		db 029h, 0C0h			; 0106  SUB  AX,AX     -> ZR
		db 048h				; 0108  DEC  AX        -> NG AC
		db 0B9h, 003h, 000h		; 0109  MOV  CX,0003
		db 001h, 0C8h			; 010C  ADD  AX,CX     -> CY
		db 0E2h, 0FCh			; 010E  LOOP 010C
		db 0F9h				; 0110  STC
		db 0BBh, 000h, 002h		; 0111  MOV  BX,0200
		db 089h, 007h			; 0114  MOV  [BX],AX   -> try D 200
		db 0EBh, 0FEh			; 0116  JMP  0116
DEMO_SIZE	equ $ - demo_code

psp_seg		dw 0
sandbox		dw 0
quit_flag	db 0

; Command letter -> handler
cmd_table	db 'R'
		dw offset cmd_r
		db 'D'
		dw offset cmd_d
		db 'E'
		dw offset cmd_e
		db 'F'
		dw offset cmd_f
		db 'U'
		dw offset cmd_u
		db 'T'
		dw offset cmd_t
		db 'H'
		dw offset cmd_h
		db '?'
		dw offset cmd_help
		db 'Q'
		dw offset cmd_q
		db 0

	.code
main proc
	mov ax,@data
	mov ds,ax
	mov psp_seg,es			; ES points at our PSP when DOS starts us

	; DOS gives an EXE all free memory.  Shrink our block to what we use
	; (PSP up to the end of the stack, which is the last segment) ...
	mov bx,sp
	mov cl,4
	shr bx,cl
	inc bx
	mov ax,ss
	add bx,ax
	sub bx,psp_seg			; BX = paragraphs used
	mov ah,4Ah
	int 21h
	jc no_memory

	; ... and allocate a separate 64 KB segment for the debuggee.
	mov bx,SANDBOX_PARAS
	mov ah,48h
	int 21h
	jc no_memory
	mov sandbox,ax

	call init_sandbox
	call install_handlers
	mov dx,offset banner
	call put_str

command_loop:
	mov dl,'-'			; DEBUG.COM's prompt
	call put_char
	mov prompt_len,1
	call read_line
	call dispatch
	cmp quit_flag,0
	je command_loop

	call remove_handlers
	mov ax,4C00h
	int 21h

no_memory:
	mov dx,offset msg_nomem
	call put_str
	mov ax,4C01h
	int 21h
main endp

;------------------------------------------------------------------------------
; init_sandbox - clear the sandbox segment, put INT 20h at offset 0 (as DOS
; does in a real PSP), copy the demo program to 0100h and point every
; segment register of the debuggee at the sandbox
;------------------------------------------------------------------------------
init_sandbox proc
	mov ax,sandbox
	mov es,ax
	xor di,di
	mov cx,8000h			; 64 KB as words
	xor ax,ax
	cld
	rep stosw

	xor di,di
	mov word ptr es:[di],20CDh	; CD 20 = INT 20h
	mov si,offset demo_code
	mov di,0100h
	mov cx,DEMO_SIZE
	rep movsb

	mov ax,sandbox
	mov r_ds,ax
	mov r_es,ax
	mov r_ss,ax
	mov r_cs,ax
	mov d_seg,ax
	mov u_seg,ax
	ret
init_sandbox endp

;------------------------------------------------------------------------------
; dispatch - run the command typed on the line at SI
;------------------------------------------------------------------------------
dispatch proc
	call skip_blanks
	or al,al
	jz dp_done			; empty line
	mov bx,offset cmd_table
dp_find:
	mov ah,[bx]
	or ah,ah
	jz dp_unknown
	cmp al,ah
	je dp_found
	add bx,3
	jmp dp_find
dp_found:
	inc si
	call word ptr [bx+1]
	ret
dp_unknown:
	mov err_pos,si
	call show_error
dp_done:
	ret
dispatch endp

cmd_help proc
	mov dx,offset help_text
	call put_str
	ret
cmd_help endp

cmd_q proc
	mov quit_flag,1
	ret
cmd_q endp

	include CONSOLE.INC
	include PARSE.INC
	include REGS.INC
	include MEMORY.INC
	include DISASM.INC
	include TRACE.INC

	end main
