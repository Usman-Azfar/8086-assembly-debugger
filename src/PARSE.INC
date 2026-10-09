;==============================================================================
; PARSE.INC - command-line parsing
;
; All parsers work on the upper-cased line returned by read_line and take
; SI as the current position, advancing it past what they consume.  On a
; syntax error they return with CF=1 and err_pos pointing at the problem.
;
; Grammar (all numbers are hexadecimal):
;   address = [segment:]offset | CS:offset | DS:offset | ES:offset | SS:offset
;   range   = end-offset | L length
;   list    = item { item }   where item = byte | "text" | 'text'
;==============================================================================

	.data
listbuf		db 84 dup(0)		; bytes collected by parse_list

	.code

;------------------------------------------------------------------------------
; skip_blanks - move SI past spaces, tabs, commas and line feeds
; Returns AL = the character at SI (0 at end of line)
;------------------------------------------------------------------------------
skip_blanks proc
sb_loop:
	mov al,[si]
	cmp al,' '
	je sb_next
	cmp al,','
	je sb_next
	cmp al,9
	je sb_next
	cmp al,10
	je sb_next
	ret
sb_next:
	inc si
	jmp sb_loop
skip_blanks endp

;------------------------------------------------------------------------------
; hex_value - convert the upper-case hex digit in AL to its value
; Returns CF=1 if AL is not a hex digit
;------------------------------------------------------------------------------
hex_value proc
	cmp al,'0'
	jb hv_bad
	cmp al,'9'
	jbe hv_digit
	cmp al,'A'
	jb hv_bad
	cmp al,'F'
	ja hv_bad
	sub al,7
hv_digit:
	sub al,'0'
	clc
	ret
hv_bad:
	stc
	ret
hex_value endp

;------------------------------------------------------------------------------
; parse_hex - read a hexadecimal number of 1 to 4 digits
; Returns AX = value, CF=1 on error
;------------------------------------------------------------------------------
parse_hex proc
	push bx
	push cx
	call skip_blanks
	mov err_pos,si
	xor bx,bx
	xor cx,cx			; digits read so far
ph_loop:
	mov al,[si]
	call hex_value
	jc ph_end
	inc cx
	cmp cx,4
	ja ph_error			; more than 4 digits
	shl bx,1
	shl bx,1
	shl bx,1
	shl bx,1
	or bl,al
	inc si
	jmp ph_loop
ph_end:
	jcxz ph_error			; no digits at all
	mov ax,bx
	clc
	jmp ph_done
ph_error:
	mov err_pos,si
	stc
ph_done:
	pop cx
	pop bx
	ret
parse_hex endp

;------------------------------------------------------------------------------
; parse_byte - read a hexadecimal number that fits in a byte
; Returns AL = value, CF=1 on error
;------------------------------------------------------------------------------
parse_byte proc
	call parse_hex
	jc pb_done
	cmp ax,0FFh
	ja pb_bad
	clc
	ret
pb_bad:
	stc
pb_done:
	ret
parse_byte endp

;------------------------------------------------------------------------------
; expect_end - make sure nothing but blanks is left on the line
; Returns CF=1 (and err_pos set) if there is trailing text.  AX is preserved.
;------------------------------------------------------------------------------
expect_end proc
	push ax
	call skip_blanks
	or al,al
	jz ee_ok
	mov err_pos,si
	pop ax
	stc
	ret
ee_ok:
	pop ax
	clc
	ret
expect_end endp

;------------------------------------------------------------------------------
; parse_address - read an address
; In:  BX = segment to use when none is typed
; Out: BX = segment, AX = offset, CF=1 on error
;------------------------------------------------------------------------------
parse_address proc
	push cx
	push di
	call skip_blanks
	mov err_pos,si
	cmp byte ptr [si+2],':'
	jne pa_number

	mov ax,[si]			; possible segment register name
	mov di,offset seg_names
	mov cx,4
pa_find:
	cmp ax,[di]
	je pa_segreg
	add di,4
	loop pa_find
	jmp pa_number

pa_segreg:
	mov di,[di+2]			; address of the register's saved value
	mov bx,[di]
	add si,3
	call parse_hex
	jmp pa_done

pa_number:
	call parse_hex
	jc pa_done
	cmp byte ptr [si],':'
	jne pa_ok
	inc si				; it was "segment:offset"
	mov bx,ax
	call parse_hex
	jmp pa_done
pa_ok:
	clc
pa_done:
	pop di
	pop cx
	ret
parse_address endp

;------------------------------------------------------------------------------
; parse_range - read the end of a range that starts at AX
; In:  AX = start offset, CX = default length (0 = a range must be typed)
; Out: CX = last offset in the range (inclusive), CF=1 on error
;      AX and BX are preserved
;------------------------------------------------------------------------------
parse_range proc
	push dx
	push ax				; [SP] = start offset
	mov dx,cx			; DX = length to use
	call skip_blanks
	mov err_pos,si
	cmp al,'L'
	je pr_length
	or al,al
	jz pr_default

	call parse_hex			; "start end" form
	jc pr_done
	mov cx,ax
	pop ax
	push ax
	cmp cx,ax
	jb pr_bad			; end before start
	clc
	jmp pr_done

pr_length:
	inc si				; "start L length" form
	call parse_hex
	jc pr_done
	mov dx,ax

pr_default:
	pop ax
	push ax
	or dx,dx
	jz pr_bad			; zero length, or no range when one is needed
	mov cx,ax
	dec dx
	add cx,dx
	jnc pr_ok
	mov cx,0FFFFh			; stop at the end of the segment
pr_ok:
	clc
	jmp pr_done
pr_bad:
	stc
pr_done:
	pop ax
	pop dx
	ret
parse_range endp

;------------------------------------------------------------------------------
; parse_list - read the rest of the line as a list of bytes and strings
; Out: listbuf = the bytes, CX = how many (at least 1), CF=1 on error
;------------------------------------------------------------------------------
parse_list proc
	push ax
	push di
	mov di,offset listbuf
	xor cx,cx
pl_loop:
	call skip_blanks
	or al,al
	jz pl_end
	mov err_pos,si
	cmp al,'"'
	je pl_string
	cmp al,27h
	je pl_string
	call parse_byte
	jc pl_done
	mov [di],al
	inc di
	inc cx
	jmp pl_loop

pl_string:
	mov ah,al			; AH = the quote character
	inc si
pl_char:
	mov al,[si]
	or al,al
	jz pl_bad			; string was never closed
	inc si
	cmp al,ah
	je pl_loop
	mov [di],al
	inc di
	inc cx
	jmp pl_char

pl_end:
	mov err_pos,si
	jcxz pl_bad			; empty list
	clc
	jmp pl_done
pl_bad:
	stc
pl_done:
	pop di
	pop ax
	ret
parse_list endp
