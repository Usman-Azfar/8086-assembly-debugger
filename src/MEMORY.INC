;==============================================================================
; MEMORY.INC - commands that work on real memory
;
;   D [address] [range]   dump memory as hex and ASCII
;   E address list        enter bytes / strings into memory
;   F address range list  fill memory with a repeating pattern
;   H value1 value2       hex arithmetic: sum and difference
;
; Addresses default to the debuggee's DS register, like DEBUG.COM.
;==============================================================================

	.data
d_seg		dw 0			; where a plain "D" continues from
d_off		dw 0100h
dm_seg		dw 0			; range being dumped
dm_start	dw 0
dm_end		dw 0

	.code

;------------------------------------------------------------------------------
; cmd_d - D [address] [range]     (default length 80h, continues where the
;                                  previous D stopped)
;------------------------------------------------------------------------------
cmd_d proc
	call skip_blanks
	mov bx,d_seg
	mov dx,d_off
	or al,al
	jz cd_range
	mov bx,r_ds
	call parse_address
	jc cd_bad
	mov dx,ax
cd_range:
	mov ax,dx
	mov cx,80h
	call parse_range
	jc cd_bad
	call expect_end
	jc cd_bad
	call dump_memory
	mov d_seg,bx
	inc cx
	mov d_off,cx
	ret
cd_bad:
	call show_error
	ret
cmd_d endp

;------------------------------------------------------------------------------
; dump_memory - print BX:AX .. BX:CX as lines of 16 bytes:
;   SSSS:OOOO  hh hh hh hh hh hh hh hh-hh hh hh hh hh hh hh hh   ascii...
; Lines are aligned on 16-byte boundaries; bytes outside the range are blank.
;------------------------------------------------------------------------------
dump_memory proc
	push ax
	push bx
	push cx
	push dx
	push di
	push es
	mov dm_seg,bx
	mov dm_start,ax
	mov dm_end,cx
	mov es,bx
	mov di,ax
	and di,0FFF0h			; DI = start of the current line

dm_line:
	mov ax,dm_seg
	call put_hex16
	mov dl,':'
	call put_char
	mov ax,di
	call put_hex16
	mov cx,2
	call put_spaces

	xor bx,bx			; BX = column 0..15
dm_hex:
	mov ax,di
	add ax,bx
	call dm_in_range
	jc dm_hex_blank
	mov al,es:[bx+di]
	call put_hex8
	jmp dm_hex_sep
dm_hex_blank:
	mov cx,2
	call put_spaces
dm_hex_sep:
	mov dl,' '
	cmp bx,7			; '-' between columns 7 and 8
	jne dm_hex_put
	mov ax,di
	add ax,7
	call dm_in_range
	jc dm_hex_put
	inc ax
	call dm_in_range
	jc dm_hex_put
	mov dl,'-'
dm_hex_put:
	call put_char
	inc bx
	cmp bx,16
	jb dm_hex

	mov cx,2
	call put_spaces
	xor bx,bx
dm_ascii:
	mov ax,di
	add ax,bx
	mov dl,' '
	call dm_in_range
	jc dm_ascii_put
	mov dl,es:[bx+di]
	cmp dl,20h
	jb dm_dot
	cmp dl,7Eh
	jbe dm_ascii_put
dm_dot:
	mov dl,'.'			; not printable
dm_ascii_put:
	call put_char
	inc bx
	cmp bx,16
	jb dm_ascii
	call put_crlf

	add di,16
	jc dm_done			; reached the end of the segment
	cmp di,dm_end
	jbe dm_line
dm_done:
	pop es
	pop di
	pop dx
	pop cx
	pop bx
	pop ax
	ret
dump_memory endp

; dm_in_range - CF=0 if offset AX is inside dm_start..dm_end, else CF=1
dm_in_range proc
	cmp ax,dm_start
	jb dr_out
	cmp ax,dm_end
	ja dr_out
	clc
	ret
dr_out:
	stc
	ret
dm_in_range endp

;------------------------------------------------------------------------------
; cmd_e - E address list      e.g.  E 200 48 69 "!" 0D 0A 24
;------------------------------------------------------------------------------
cmd_e proc
	mov bx,r_ds
	call parse_address
	jc ce_bad
	push ax
	push bx
	call parse_list			; CX = number of bytes
	pop bx
	pop di
	jc ce_bad
	mov es,bx
	mov si,offset listbuf
	cld
	rep movsb
	ret
ce_bad:
	call show_error
	ret
cmd_e endp

;------------------------------------------------------------------------------
; cmd_f - F address range list    e.g.  F 200 L 40 "AB"   or   F 200 2FF 0
;------------------------------------------------------------------------------
cmd_f proc
	mov bx,r_ds
	call parse_address
	jc cf_bad
	xor cx,cx			; a range is required
	call parse_range
	jc cf_bad
	push ax
	push bx
	push cx
	call parse_list
	mov dx,cx			; DX = pattern length
	pop cx				; CX = last offset
	pop bx
	pop di				; DI = first offset
	jc cf_bad
	mov es,bx
	xor bx,bx			; BX = position in the pattern
cf_loop:
	mov al,listbuf[bx]
	mov es:[di],al
	inc bx
	cmp bx,dx
	jb cf_next
	xor bx,bx			; repeat the pattern
cf_next:
	cmp di,cx
	je cf_done
	inc di
	jmp cf_loop
cf_done:
	ret
cf_bad:
	call show_error
	ret
cmd_f endp

;------------------------------------------------------------------------------
; cmd_h - H value1 value2     prints value1+value2 and value1-value2
;------------------------------------------------------------------------------
cmd_h proc
	call parse_hex
	jc ch_bad
	mov bx,ax
	call parse_hex
	jc ch_bad
	mov dx,ax
	call expect_end
	jc ch_bad
	mov ax,bx
	add ax,dx
	call put_hex16
	mov cx,2
	call put_spaces
	mov ax,bx
	sub ax,dx
	call put_hex16
	call put_crlf
	ret
ch_bad:
	call show_error
	ret
cmd_h endp
