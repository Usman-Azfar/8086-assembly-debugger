;==============================================================================
; TRACE.INC - the T command: real single-stepping with the 8086 Trap Flag
;
; How one step works:
;   1. step_launch saves the debugger's own SS:SP, switches to the debuggee's
;      stack and builds an interrupt frame there: FLAGS (with TF=1), CS, IP.
;   2. It loads every debuggee register into the CPU and executes IRET.
;      The CPU jumps to CS:IP and runs exactly one instruction.
;   3. Because TF was set, the CPU then raises INT 1.  trap_handler saves
;      all registers back into the register file, restores the debugger's
;      stack and returns as if step_launch had been an ordinary call.
;
; Interrupt instructions need special care: the CPU clears TF when it enters
; an interrupt handler, and the trap only fires after the instruction that
; follows the handler's IRET, so one step would run two instructions.  For
; INT nn and INTO the debugger therefore plants a temporary INT 3 breakpoint
; (byte CCh) on the next instruction and runs without TF instead.  The
; handler runs at full speed and the breakpoint stops the program right after
; it returns.  INT 1 and INT 3 both go to trap_handler.
;==============================================================================

	.data
our_ss		dw 0			; debugger stack while the debuggee runs
our_sp		dw 0
old_int1	dd 0			; previous vectors, restored on exit
old_int3	dd 0
bp_active	db 0			; 1 = a temporary breakpoint is planted
bp_seg		dw 0			; where it is
bp_off		dw 0
bp_saved	db 0			; the byte it replaced
msg_blocked	db "Not traced: that instruction would end the debugger itself", 13, 10, "$"

	.code

;------------------------------------------------------------------------------
; install_handlers / remove_handlers - hook INT 1 (single step), INT 3
; (breakpoint) and INT 23h (Ctrl-C).  DOS restores INT 23h by itself when
; the program ends.
;------------------------------------------------------------------------------
install_handlers proc
	push ax
	push bx
	push dx
	push ds
	push es
	mov ax,3501h			; get INT 1 vector
	int 21h
	mov word ptr old_int1,bx
	mov word ptr old_int1+2,es
	mov ax,3503h			; get INT 3 vector
	int 21h
	mov word ptr old_int3,bx
	mov word ptr old_int3+2,es
	push cs
	pop ds
	mov dx,offset trap_handler
	mov ax,2501h			; set INT 1 vector
	int 21h
	mov ax,2503h			; set INT 3 vector
	int 21h
	mov dx,offset ctrlc_handler
	mov ax,2523h			; set INT 23h vector
	int 21h
	pop es
	pop ds
	pop dx
	pop bx
	pop ax
	ret
install_handlers endp

remove_handlers proc
	push ax
	push dx
	push ds
	push ds
	lds dx,old_int1
	mov ax,2501h
	int 21h
	pop ds
	lds dx,old_int3
	mov ax,2503h
	int 21h
	pop ds
	pop dx
	pop ax
	ret
remove_handlers endp

; Ctrl-C / Ctrl-Break: ignore it, so the debugger can't be left with
; its INT 1 hook still installed
ctrlc_handler proc
	iret
ctrlc_handler endp

;------------------------------------------------------------------------------
; cmd_t - T [count]    execute count instructions (default 1), showing the
;                      registers after each one
;------------------------------------------------------------------------------
cmd_t proc
	call skip_blanks
	mov cx,1
	or al,al
	jz ct_loop
	call parse_hex
	jc ct_bad
	or ax,ax
	jz ct_bad_count
	mov cx,ax
	call expect_end
	jc ct_bad
ct_loop:
	call check_terminate
	jc ct_blocked
	call single_step
	call show_regs
	loop ct_loop
	jmp ct_sync
ct_blocked:
	mov dx,offset msg_blocked
	call put_str
ct_sync:
	mov ax,r_cs			; a plain U now starts at the new CS:IP
	mov u_seg,ax
	mov ax,r_ip
	mov u_off,ax
	ret
ct_bad_count:
	stc
ct_bad:
	call show_error
	ret
cmd_t endp

;------------------------------------------------------------------------------
; check_terminate - CF=1 if the instruction at CS:IP is INT 20h, INT 27h, or
; INT 21h with AH = 00h / 31h / 4Ch.  Executing one of these would end the
; debugger itself, because DOS sees it as the running program.
;------------------------------------------------------------------------------
check_terminate proc
	push ax
	push bx
	push es
	mov es,r_cs
	mov bx,r_ip
	cmp byte ptr es:[bx],0CDh	; INT nn ?
	jne ctm_ok
	mov al,es:[bx+1]
	cmp al,20h
	je ctm_block
	cmp al,27h
	je ctm_block
	cmp al,21h
	jne ctm_ok
	mov al,byte ptr r_ax+1		; AH of the debuggee
	cmp al,00h
	je ctm_block
	cmp al,31h
	je ctm_block
	cmp al,4Ch
	je ctm_block
ctm_ok:
	clc
	jmp ctm_done
ctm_block:
	stc
ctm_done:
	pop es
	pop bx
	pop ax
	ret
check_terminate endp

;------------------------------------------------------------------------------
; single_step - execute one debuggee instruction.  All registers preserved.
;------------------------------------------------------------------------------
single_step proc
	push ax
	push bx
	push cx
	push dx
	push si
	push di
	push bp
	push es

	mov es,r_cs			; is it INT nn or INTO?
	mov bx,r_ip
	mov al,es:[bx]
	mov cx,2
	cmp al,0CDh
	je ss_breakpoint
	mov cx,1
	cmp al,0CEh
	jne ss_launch
ss_breakpoint:
	add bx,cx			; BX = the instruction after it
	mov bp_seg,es
	mov bp_off,bx
	mov al,es:[bx]
	mov bp_saved,al
	mov byte ptr es:[bx],0CCh	; INT 3
	mov bp_active,1
ss_launch:
	call step_launch		; "returns" here from trap_handler
	pop es
	pop bp
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	ret
single_step endp

step_launch proc
	mov our_ss,ss
	mov our_sp,sp			; top of stack = return address into single_step

	cli
	mov ss,r_ss			; switch to the debuggee's stack
	mov sp,r_sp
	mov ax,r_fl
	and ax,not TRAP_FLAG
	cmp bp_active,0
	jne sl_frame			; breakpoint planted: run without TF
	or ax,TRAP_FLAG
sl_frame:
	push ax				; interrupt frame for IRET
	push r_cs
	push r_ip
	push r_ds			; popped into DS last

	mov ax,r_ax
	mov bx,r_bx
	mov cx,r_cx
	mov dx,r_dx
	mov bp,r_bp
	mov si,r_si
	mov di,r_di
	mov es,r_es
	pop ds
	iret				; run one instruction, then INT 1 or 3
step_launch endp

;------------------------------------------------------------------------------
; trap_handler - entry point for INT 1 (single step) and INT 3 (breakpoint)
;------------------------------------------------------------------------------
trap_handler proc
	push ds				; on entry: debuggee's registers,
	push ax				; stack holds IP, CS, FLAGS
	mov ax,@data
	mov ds,ax
	pop ax
	mov r_ax,ax
	pop ax
	mov r_ds,ax
	pop ax
	mov r_ip,ax
	pop ax
	mov r_cs,ax
	pop ax
	and ax,not TRAP_FLAG
	mov r_fl,ax
	mov r_bx,bx
	mov r_cx,cx
	mov r_dx,dx
	mov r_bp,bp
	mov r_si,si
	mov r_di,di
	mov r_es,es
	mov r_ss,ss
	mov r_sp,sp			; frame removed: this is the debuggee's SP

	mov ss,our_ss			; back to the debugger's stack
	mov sp,our_sp
	sti
	cld

	cmp bp_active,0			; remove a planted breakpoint
	je th_done
	mov bp_active,0
	mov es,bp_seg
	mov bx,bp_off
	mov al,bp_saved
	mov es:[bx],al
	mov ax,r_cs			; stopped by it?  Then IP points one
	cmp ax,bp_seg			; past it: move IP back onto the
	jne th_done			; restored instruction
	mov ax,r_ip
	dec ax
	cmp ax,bp_off
	jne th_done
	mov r_ip,ax
th_done:
	ret				; to single_step, right after "call step_launch"
trap_handler endp
