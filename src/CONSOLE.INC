;==============================================================================
; CONSOLE.INC - character input/output on top of DOS INT 21h
;
; Every routine preserves all registers except the ones it returns a value in.
; Output normally goes to the screen.  When cap_ptr is non-zero, characters
; are captured into memory at that address instead; the disassembler uses this
; to build a line of text before printing it.
;==============================================================================

	.data
cap_ptr		dw 0			; 0 = print to screen, else capture address
prompt_len	db 1			; width of the prompt in front of the input line
err_pos		dw 0			; position in inbuf where a parse error was found
inbuf		db 80, 0, 84 dup(0)	; buffered-input block for INT 21h/0Ah
err_msg		db "^ Error", 13, 10, "$"

	.code

;------------------------------------------------------------------------------
; put_char - write the character in DL
;------------------------------------------------------------------------------
put_char proc
	push ax
	push di
	mov di,cap_ptr
	or di,di
	jnz pc_capture
	mov ah,02h
	int 21h
	jmp pc_done
pc_capture:
	mov [di],dl
	inc cap_ptr
pc_done:
	pop di
	pop ax
	ret
put_char endp

;------------------------------------------------------------------------------
; put_str - write the '$'-terminated string at DS:DX
;------------------------------------------------------------------------------
put_str proc
	push dx
	push si
	mov si,dx
ps_loop:
	mov dl,[si]
	cmp dl,'$'
	je ps_done
	call put_char
	inc si
	jmp ps_loop
ps_done:
	pop si
	pop dx
	ret
put_str endp

;------------------------------------------------------------------------------
; put_crlf - move to the start of the next line
;------------------------------------------------------------------------------
put_crlf proc
	push dx
	mov dl,13
	call put_char
	mov dl,10
	call put_char
	pop dx
	ret
put_crlf endp

;------------------------------------------------------------------------------
; put_spaces - write CX spaces (CX may be 0)
;------------------------------------------------------------------------------
put_spaces proc
	push cx
	push dx
	mov dl,' '
	jcxz psp_done
psp_loop:
	call put_char
	loop psp_loop
psp_done:
	pop dx
	pop cx
	ret
put_spaces endp

;------------------------------------------------------------------------------
; put_hex16 / put_hex8 / put_nibble - write AX, AL or the low nibble of AL
; as upper-case hexadecimal digits
;------------------------------------------------------------------------------
put_hex16 proc
	push ax
	xchg al,ah
	call put_hex8
	xchg al,ah
	call put_hex8
	pop ax
	ret
put_hex16 endp

put_hex8 proc
	push ax
	push cx
	push ax
	mov cl,4
	shr al,cl
	call put_nibble
	pop ax
	call put_nibble
	pop cx
	pop ax
	ret
put_hex8 endp

put_nibble proc
	push ax
	push dx
	and al,0Fh
	add al,'0'
	cmp al,'9'
	jbe pn_out
	add al,7			; 'A'..'F'
pn_out:
	mov dl,al
	call put_char
	pop dx
	pop ax
	ret
put_nibble endp

;------------------------------------------------------------------------------
; read_line - read one line of input with DOS line editing (backspace works)
;
; Returns SI -> the text, terminated by a 0 byte.  Letters are converted to
; upper case, except inside '...' or "..." so strings typed for the E and F
; commands keep their case.
;------------------------------------------------------------------------------
read_line proc
	push ax
	push bx
	push dx
	mov dx,offset inbuf
	mov ah,0Ah
	int 21h
	call put_crlf
	mov bl,inbuf+1			; number of characters typed
	xor bh,bh
	mov si,offset inbuf+2
	mov byte ptr [si+bx],0		; replace the CR with a terminator

	xor ah,ah			; AH = open quote character, 0 = none
rl_loop:
	mov al,[si]
	or al,al
	jz rl_done
	or ah,ah
	jnz rl_in_quote
	cmp al,'"'
	je rl_open_quote
	cmp al,27h			; single quote
	je rl_open_quote
	cmp al,'a'
	jb rl_next
	cmp al,'z'
	ja rl_next
	sub al,20h			; to upper case
	mov [si],al
	jmp rl_next
rl_open_quote:
	mov ah,al
	jmp rl_next
rl_in_quote:
	cmp al,ah
	jne rl_next
	xor ah,ah			; closing quote
rl_next:
	inc si
	jmp rl_loop
rl_done:
	mov si,offset inbuf+2
	pop dx
	pop bx
	pop ax
	ret
read_line endp

;------------------------------------------------------------------------------
; show_error - print "^ Error" with the caret under the position in err_pos,
; the same way DEBUG.COM points at the bad part of a command
;------------------------------------------------------------------------------
show_error proc
	push ax
	push cx
	push dx
	mov cx,err_pos
	sub cx,offset inbuf+2
	mov al,prompt_len
	xor ah,ah
	add cx,ax
	call put_spaces
	mov dx,offset err_msg
	call put_str
	pop dx
	pop cx
	pop ax
	ret
show_error endp
