.model small
.stack 100h
.386
.data
	
	;String for Input Command Display:
	
	input_command db 10,13, "Press r/R (register command) to start or q/Q (exit command) to exit : $"
	
	; String to display info message to user:
	
	user_info db "---Note : All the commands used in this program are case Insensitive---$"

	;Strings to print Register Names : 
	
	str1  db  10,13, 	 "AX=$"
	str2  db  20h,20h,   "BX=$"
	str3  db  20h,20h,   "CX=$"
	str4  db  20h,20h,   "DX=$"
	str5  db  20h,20h,   "SP=$"
	str6  db  20h,20h,   "BP=$"
	str7  db  20h,20h,   "SI=$"
	str8  db  20h,20h,   "DI=$"
	str9  db  10,13, 	 "DS=$"
	str10 db  20h,20h,   "ES=$"
	str11 db  20h,20h,   "SS=$"
	str12 db  20h,20h,   "CS=$"
	str13 db  20h,20h,   "IP=$"
	
		
	;Strings to display flag values : 

	str15 db 20h,20h, "OV UP EI NG ZR NA PO CY$"	;Z C S O
	str16 db 20h,20h, "NV UP EI NG ZR NA PO CY$"	;Z C S 
	str17 db 20h,20h, "OV UP EI PL ZR NA PO CY$"	;Z C O
	str18 db 20h,20h, "NV UP EI PL ZR NA PO CY$"	;Z C 
	str19 db 20h,20h, "OV UP EI NG ZR NA PO NC$"	;Z S O
	str20 db 20h,20h, "NV UP EI NG ZR NA PO NC$"	;Z S
	str21 db 20h,20h, "OV UP EI PL ZR NA PO NC$"	;Z O
	str22 db 20h,20h, "NV UP EI PL ZR NA PO NC$"	;Z 
	str23 db 20h,20h, "OV UP EI NG NZ NA PO CY$"	;C S O 
	str24 db 20h,20h, "NV UP EI NG NZ NA PO CY$"	;C S 
	str25 db 20h,20h, "OV UP EI PL NZ NA PO CY$"	;C O 
	str26 db 20h,20h, "NV UP EI PL NZ NA PO CY$"	;C 
	str27 db 20h,20h, "OV UP EI NG NZ NA PO NC$"	;S O 
	str28 db 20h,20h, "NV UP EI NG NZ NA PO NC$"	;S 
	str29 db 20h,20h, "OV UP EI PL NZ NA PO NC$"	;O 
	str30 db 20h,20h, "NV UP EI PL NZ NA PO NC$"	;No Flag Active
	
	;String to Store Current FLag Values: 
	
	flag_string db 50 dup("$") 	
	
	;Strings to modify register values: 
	
	str_r  db "r$"
	str_q  db "q$"
	str_ax db "r ax$"
	str_bx db "r bx$"
	str_cx db "r cx$"
	str_dx db "r dx$"
	str_sp db "r sp$"
	str_bp db "r bp$"
	str_si db "r si$"
	str_di db "r di$"
	str_ds db "r ds$"
	str_es db "r es$"
	str_ss db "r ss$"
	str_cs db "r cs$"
	str_ip db "r ip$"
	
	;String to Store user input to modify register values:
	
	register_input  db 20 dup("$")
	
	;Variables to store register Values: 
	
	var1  dw 0		;STORES AX
	var2  dw 0		;STORES BX
	var3  dw 0		;STORES CX
	var4  dw 0		;STORES DX
	var5  dw 0		;STORES SP
	var6  dw 0		;STORES BP
	var7  dw 0		;STORES SI
	var8  dw 0		;STORES DI
	var9  dw 0		;STORES DS
	var10 dw 0		;STORES ES
	var11 dw 0		;STORES SS
	var12 dw 0		;STORES CS
	var13 dw 0		;STORES IP
	
	;Variables to use for flag statuses :
	
	var20 db 0
	var21 db 0
	var22 db 0
	var23 db 0
	var24 db 0
	var25 db 0
	var26 db 0
	var27 db 0
	var28 db 0
	var29 db 0
	var30 db 0
	var31 db 0
	var32 db 0
	var33 db 0
	var34 db 0
	var35 db 0
	
	;Variables to check active flags:
	
	zero_var db 0
	carry_var db 0
	sign_var db 0
	overflow_var db 0			

	;Strings to display previous register values:
	
	str35  db  "AX $"
	str36  db  "BX $"
	str37  db  "CX $"
	str38  db  "DX $"
	str39  db  "SP $"
	str40  db  "BP $"
	str41  db  "SI $"
	str42  db  "DI $"
	str43  db  "DS $"
	str44  db  "ES $"
	str45  db  "SS $"
	str46  db  "CS $"
	str47  db  "IP $"
	
	;Variable to store input:
	
	input_store 	 db 0		
	
	;Variable that is incremented when first input is entered:
	
	input_counter    db 0
	
	;String to display error message:
	
	error_string     db "^ Error$"
	
	;String to display Next Line:
	
	next_line_string db 10,13, "$"
	
	;Next Instruction String:
	
	next_instruction db "ADD   [BX+SI],AL$"	
	
	;Variable to check if register_input string has values more than 4:
	
	var_str db 0
	
	;Variable to check if strings are equal:
	
	str_equal db 0
	
	;Variable to check if a Hex digit is Illegal:
	
	illegal_digit db 0
	
	;Variable to check if Hex input is more than 4 digits:
	
	hex_var db 0
	
	;Variable to check if register input is changed/modified:
	
	hex_input_active db 0
	
	;Variable to check if input is r or q:s
	
	r_q_input db 0
	
	;Variable to handle all registers reset(0000) values: 
	
	first_input_check db 0
	
	; Variable to handle enter error in hexa input:
	
	enter_check db 0
	
	;temporary Variable:
	
	 temp_var dw 0
	 
	;Register Command string: 
	register_cmd_print db 10,13, "(REGISTER COMMAND)$"
	
	;Exit Command: 
	exit_cmd_print db 10,13, "(EXIT COMMAND)$"
	
.code
	main proc 
		mov ax,@data
		mov ds,ax
		mov es,ax
		
		;Display Info Message to user:
		
		call NextLine
		call Give_Space
		call Give_Space
		call Give_Space
		lea dx,user_info
		mov ah,9
		int 21h
		call NextLine
		
		;Display Input Command:

		lea dx,input_command
		mov ah,09
		int 21h
		
first_input:
		mov ah,01
		int 21h
		mov first_input_check,1		
		cmp al,13
		je errr
		mov input_store,al
		inc input_counter
		
	
		mov ah,01
		int 21h
		inc input_counter
		cmp al,13
		je input_check 
		jmp first_input
		

		
input_check:
		cmp input_counter,2				;If input has 2 digits
		je input_correct 
		call Errorr						;If input has more than 2 digits it is error
		call NextLine
		mov input_counter,0
		jmp first_input
		
input_correct:
		mov input_counter,0
		mov first_input_check,0
		cmp input_store,"q"				
		je end_program
		cmp input_store,"Q"				
		je end_program	
		cmp input_store,"r"
		je register_command
		cmp input_store,"R"
		je register_command
		call Errorr
		call NextLine
		jmp first_input


register_command:


		
		;Variables storing initial values of registers:
		
		cmp hex_input_active,1
		je print_register
		
		;Operation to check flag values: 
		
		mov ax,7fffh
		add ax,7fffh
		add ax,bx
		
		call checkFLag					;Checking Flag Values
		
		mov var1,ax
		mov var2,bx
		mov var3,cx
		mov var4,dx
		mov var5,sp
		mov var6,bp
		mov var7,si
		mov var8,di
		mov var9,ds
		mov var10,es
		mov var11,ss
		mov var12,cs
		lea ax,[ip_value]				;getting IP Value
		mov var13,ax
				;Operation to check flag values: 
		
		mov ax,7fffh
		add ax,7fffh
		
		;add ax,bx
		

		call checkFLag					;Checking Flag Values
			
						
		;Printing values of registers: 
		
print_register:	
		call Print_Register_Command
		call NextLine
		
		lea dx,str1			
		mov ah,9
		int 21h
		mov bx,var1			;AX
		call hexadecimalOutput
		lea dx,str2
		mov ah,9
		int 21h
		mov bx,var2			;BX
		call hexadecimalOutput
		lea dx,str3
		mov ah,9
		int 21h
		mov bx,var3			;CX
		call hexadecimalOutput
		lea dx,str4
		mov ah,9
		int 21h
		mov bx,var4			;DX
		call hexadecimalOutput
		
		lea dx,str5
		mov ah,9
		int 21h
		mov bx,var5			;SP
		call hexadecimalOutput
		lea dx,str6
		mov ah,9
		int 21h
		mov bx,var6			;BP
		call hexadecimalOutput
		lea dx,str7
		mov ah,9
		int 21h
		mov bx,var7			;SI
		call hexadecimalOutput
		lea dx,str8
		mov ah,9
		int 21h
		mov bx,var8			;DI
		call hexadecimalOutput
		lea dx,str9
		mov ah,9
		int 21h
		mov bx,var9			;DS
		call hexadecimalOutput
		lea dx,str10
		mov ah,9
		int 21h
		mov bx,var10		;ES
		call hexadecimalOutput
		lea dx,str11
		mov ah,9
		int 21h
		mov bx,var11		;SS
		call hexadecimalOutput
		lea dx,str12
		mov ah,9
		int 21h
		mov bx,var12		;CS
		call hexadecimalOutput
		lea dx,str13
		mov ah,9
		int 21h
		mov bx,var13		;IP
		call hexadecimalOutput
		call Give_Space
		lea dx,flag_string
		mov ah,9
		int 21h
		call NextLine
		call Segment_Offset_Address_Print
		call NextLine

		
		;Taking user input in register_input string:
		
str_input:
		call empty_string		;Making input string empty first
		lea di,register_input
		mov var_str,0
		mov str_equal,0
		mov r_q_input,0
again_strinput:
		mov ah,01h
		int 21h
		cmp al,13
		je callcomp			;Compare strings when enter is pressed
		stosb
		inc var_str
		inc r_q_input
		cmp var_str,4
		ja error_enter
		jmp again_strinput
		
		
	;If register_input string has more than 4 values print error:
	
error_enter:
		mov ah,01
		int 21h
		cmp al,13
		je errr
		jmp error_enter

	;Printing Error String:
	
errr:  	call Errorr
		call NextLine
		mov input_counter,0
		mov illegal_digit,0
		mov hex_var,0
		cmp first_input_check,1	
		je first_input			
		jmp str_input	

	;Strings Comparison:
		
callcomp:
		cmp r_q_input,1		;If user input string contains single r or q
		je goto_r_q
		
		
		lea si,register_input	;Comparing user string with AX
		lea di,str_ax
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je ax_input

		
		lea si,register_input	;Comparing user string with BX
		lea di,str_bx
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je bx_input
		
		
		lea si,register_input	;Comparing user string with CX
		lea di,str_cx
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je cx_input
		
		lea si,register_input	;Comparing user string with DX
		lea di,str_dx
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je dx_input
		
		lea si,register_input	;Comparing user string with SP
		lea di,str_sp
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je sp_input
		
		lea si,register_input	;Comparing user string with BP
		lea di,str_bp
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je bp_input
		
		lea si,register_input	;Comparing user string with SI
		lea di,str_si
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je si_input
		
		lea si,register_input	;Comparing user string with DI
		lea di,str_di
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je di_input
		
		lea si,register_input	;Comparing user string with DS
		lea di,str_ds
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je ds_input
		
		lea si,register_input	;Comparing user string with ES
		lea di,str_es
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je es_input
		
		lea si,register_input	;Comparing user string with SS
		lea di,str_ss
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je ss_input
		
		lea si,register_input	;Comparing user string with CS
		lea di,str_cs
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je cs_input
		
		lea si,register_input	;Comparing user string with IP
		lea di,str_ip
		mov cx,4
		call compare_strings
		cmp str_equal,1
		je ip_input
		jmp errr

;Moving to register command or exit command according to user input:
		
goto_r_q:
	mov r_q_input,0
	
	lea si,register_input
	mov al,"r"
	cmp [si],al
	je	print_register
	mov al,"R"
	cmp [si],al
	je	print_register
		
	mov al,"q"
	cmp [si],al
	je end_program
	mov al,"Q"
	cmp [si],al
	je end_program
	jmp errr

;Changing AX value:
ax_input:
		lea dx,str35
		mov ah,09
		int 21h
		mov bx,var1
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var1
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var1,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
	    jmp str_input

;Changing BX value:
bx_input:

		lea dx,str36
		mov ah,09
		int 21h
		mov bx,var2
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var2
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var2,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp str_input


;Changing CX value:
cx_input:

		lea dx,str37
		mov ah,09
		int 21h
		mov bx,var3
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var3
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var3,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp str_input

;Changing DX value:
dx_input:

		lea dx,str38
		mov ah,09
		int 21h
		mov bx,var4
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var4
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var4,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp str_input

;Changing SP value:
sp_input:

		lea dx,str39
		mov ah,09
		int 21h
		mov bx,var5
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var5
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var5,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp str_input

;Changing BP value:
bp_input:

		lea dx,str40
		mov ah,09
		int 21h
		mov bx,var6
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var6
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var6,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp str_input

;Changing SI value:
si_input:

		lea dx,str41
		mov ah,09
		int 21h
		mov bx,var7
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var7
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var7,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp str_input

;Changing DI value:
di_input:

		lea dx,str42
		mov ah,09
		int 21h
		mov bx,var8
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var8
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var8,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp str_input

;Changing DS value:
ds_input:

		lea dx,str43
		mov ah,09
		int 21h
		mov bx,var9
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var9
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var9,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp str_input

;Changing ES value:
es_input:

		lea dx,str44
		mov ah,09
		int 21h
		mov bx,var10
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var10
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var10,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp str_input

;Changing SS value:
ss_input:
		lea dx,str45
		mov ah,09
		int 21h
		mov bx,var11
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var11
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var11,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp str_input

;Changing CS value:
cs_input:

		lea dx,str46
		mov ah,09
		int 21h
		mov bx,var12
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var12
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var12,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp	str_input

;Changing IP value:
ip_input:

		lea dx,str47
		mov ah,09
		int 21h
		mov bx,var13
		call hexadecimalOutput
		call NextLine
		call print_colon
		mov dx,var13
		mov temp_var,dx
		call hexadecimalInput
		cmp illegal_digit,1
		je 	errr
		mov var13,bx
		mov illegal_digit,0
		mov hex_var,0
		mov hex_input_active,1
		jmp str_input

	;Program Exit:	
	
end_program:

	call Print_Exit_Command
	call NextLine
	
	mov ah,4ch
	int 21h
	
ip_value:		;To get IP value
	nop
	main endp
	
;Function to Print colon: 
	
	print_colon proc
	
		mov dl,':'
		mov ah,02
		int 21h
		ret
	print_colon endp	
	
;Function to compare Strings: 

	compare_strings proc

compare_start:	
	mov al,[di]
	mov bl,[si]
	
	cmp bl,"R"
	je change_R
	cmp bl,"A"
	je change_A
	cmp bl,"B"
	je change_B
	cmp bl,"C"
	je change_C
	cmp bl,"D"
	je change_D
	cmp bl,"E"
	je change_E
	cmp bl,"I"
	je change_I
	cmp bl,"P"
	je change_P
	cmp bl,"S"
	je change_S
	cmp bl,"X"
	je change_X
	
	
do_comparison:
	cmp al,bl
	jne exit_cmp_strings
	inc si
	inc di
	dec cx
	jnz compare_start
	inc str_equal
	jmp exit_cmp_strings
	
change_R:
	mov bl,"r"
	jmp do_comparison

change_A:
	mov bl,"a"
	jmp do_comparison
	
change_B:
	mov bl,"b"
	jmp do_comparison
	
change_C:
	mov bl,"c"
	jmp do_comparison
	
change_D:
	mov bl,"d"
	jmp do_comparison
	
change_E:
	mov bl,"e"
	jmp do_comparison
	
change_I:
	mov bl,"i"
	jmp do_comparison
	
change_P:
	mov bl,"p"
	jmp do_comparison
	
change_S:
	mov bl,"s"
	jmp do_comparison
	
change_X:
	mov bl,"x"
	jmp do_comparison
				
exit_cmp_strings:

	ret
 
	compare_strings endp

;Function to get output in HexaDecimal Form:

	hexadecimalOutput proc
        
		mov  cl,4
							;Value to be printed in hexa must be in BX
hexa_input: 
		 rol  bx,4
         mov  dl,bl
         and  dl,0fh
         cmp  dl,9
         jbe  digit
         add  dl,7h
digit:   
		 add  dl,30h
         mov  ah,02h
         int  21h
         dec  cl
         jnz  hexa_input
         
		 ret
		 
hexadecimalOutput endp

;Function to get Input in HexaDecimal Form:

	hexadecimalInput proc
		xor bx,bx
			
start_hex_input:
		mov ah,1
		int 21h
		inc enter_check	;
		cmp al,13
		je return_hex
		cmp al,'0'
		jae check_hex
		cmp al,'A'
		jae check_hex
		mov illegal_digit,1
		jmp start_hex_input
		
check_hex:
		cmp al,'a'
		jae check_hex2
		cmp al,'F'
		jbe correct_hex
		cmp al,'9'
		jbe correct_hex
		mov illegal_digit,1
		jmp start_hex_input

check_hex2:
		cmp al,'f'
		jbe convert_to_capital
		mov illegal_digit,1
		jmp start_hex_input
		
;Tag to Handle small Alphabets

convert_to_capital:
	cmp al,'a'
	jne b_check
	mov al,'A'
	jmp correct_hex
b_check:
	cmp al,'b'
	jne c_check
	mov al,'B'
	jmp correct_hex
c_check:
	cmp al,'c'
	jne d_check
	mov al,'C'
	jmp correct_hex
d_check:
	cmp al,'d'
	jne e_check
	mov al,'D'
	jmp correct_hex
e_check:
	cmp al,'e'
	jne f_check
	mov al,'E'
	jmp correct_hex
f_check:
	cmp al,'f'
	jne correct_hex
	mov al,'F'	
	
		
correct_hex:
		cmp al,'9'
		jbe store_hex
		sub al,7h
		
store_hex:
		sub al,30h				;Hex is stored in BX
		shl bx,4
		or bl,al
		inc hex_var
		cmp hex_var,4
		ja active_illegal_digit
do_again:		
		jmp start_hex_input
		
active_illegal_digit:     		;to handle an illegal hex digit
	mov illegal_digit,1
	jmp do_again
		
return_hex:
	cmp enter_check,1	;
	jne	exit_hexa_input
	mov bx,temp_var
	
exit_hexa_input:
	mov enter_check,0
	
		ret
		
	hexadecimalInput endp

;Function to print Error message: 

	Errorr proc
	
	lea dx,error_string
	mov ah,9
	int 21h
	
	ret
	Errorr endp

;Function to print next line:
	
	NextLine proc
	
	lea dx,next_line_string
	mov ah,9
	int 21h
	
	ret
	
	NextLine endp

;Function to give space:
	
	Give_Space proc
	
	mov dl,20h
	mov ah,2
	int 21h
	
	ret
	
	Give_Space endp

;Function to make register_input string empty:	

	empty_string proc
	
	mov cx,20
	lea si,register_input
	
again_empty:
	mov al,'$'
	mov [si],al
	inc si
	dec cx
	jnz again_empty
	ret
	
	empty_string endp
	
;Function to print Segment and Offset Address:
		
	Segment_Offset_Address_Print proc
	
	mov bx,var9				;Printing value of DS
	call hexadecimalOutput
	mov dl,":"
	mov ah,2
	int 21h
	mov bx,var13			;Printing value of IP
	call hexadecimalOutput
	call Give_Space
	mov bx,0000h			;Printing Default Machine Code
	call hexadecimalOutput
	
	call Give_Space
	call Give_Space
	call Give_Space
	call Give_Space
	call Give_Space
	call Give_Space
	call Give_Space
	call Give_Space
	call Give_Space
	
	lea dx,next_instruction		;Printing Next Instruction
	mov ah,9
	int 21h
	
	ret
	
	Segment_Offset_Address_Print endp
	
;Function to check Flag Values: 

	checkFLag proc

	jz zero_active
zero_ret:
	jc carry_active
carry_ret:
	js	sign_active
sign_ret:
	jo overflow_active
	jmp start_check


zero_active:
	mov zero_var,1
	jmp zero_ret
carry_active:
	mov carry_var,1
	jmp carry_ret
sign_active:
	mov sign_var,1
	jmp sign_ret
overflow_active:
	mov overflow_var,1


start_check:
	cmp zero_var,1
	je zero
	jmp notzero

zero:
	inc var20
	inc var21
	inc var22
	inc var23
	inc var24
	inc var25
	inc var26
	inc var27
	cmp carry_var,1
	je carry
	jmp notcarry
	
carry:
	inc var20
	inc var21
	inc var22
	inc var23
	inc var28
	inc var29
	inc var30
	inc var31
	cmp sign_var,1
	je sign						;if negative answer
	jmp notsign
	
sign:
	inc var20
	inc var21
	inc var24
	inc var25
	inc var28
	inc var29
	inc var32
	inc var33
	cmp overflow_var,1
	je overflow
	jmp notoverflow
	
overflow:
	inc var20
	inc var22
	inc var24
	inc var26
	inc var28
	inc var30
	inc var32
	inc var34
	jmp exit_flag
	
notzero:
	cmp carry_var,1
	je carry
	jmp notcarry

notcarry:
	cmp sign_var,1
	je sign
	jmp notsign
	
notsign:
	cmp overflow_var,1
	je overflow
	jmp notoverflow
	
notoverflow:
	jmp exit_flag



exit_flag:
	
	cmp var20,4
	je case1
	cmp var21,3
	je case2
	cmp var22,3
	je case3
	cmp var23,2
	je case4
	cmp var24,3
	je case5
	cmp var25,2
	je case6
	cmp var26,2
	je case7
	cmp var27,1
	je case8
	cmp var28,3
	je case9
	cmp var29,2
	je case10
	cmp var30,2
	je case11
	cmp var31,1
	je case12
	cmp var32,2
	je case13
	cmp var33,1
	je case14
	cmp var34,1
	je case15
	cmp var35,0
	je case16
	jmp exit_function


case1:
	lea si,str15
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case2:
	lea si,str16
	lea di,flag_string
	mov cx,25
	cld
	
	rep movsb
	
	jmp exit_function

case3:
	lea si,str17
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case4:
	lea si,str18
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case5:
	lea si,str19
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case6:
	lea si,str20
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case7:
	lea si,str21
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case8:
	lea si,str22
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case9:
	lea si,str23
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case10:
	lea si,str24
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case11:
	lea si,str25
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case12:
	lea si,str26
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case13:
	lea si,str27
	lea di,flag_string
	mov cx,25
	cld															

	rep movsb
	
	jmp exit_function

case14:
	lea si,str28
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case15:
	lea si,str29
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

case16:
	lea si,str30
	lea di,flag_string
	mov cx,25
	cld

	rep movsb
	
	jmp exit_function

exit_function:	
	;lea dx,flag_string
	;mov ah,9
	;int 21h
	
	
	mov var20,0
	mov var21,0
	mov var22,0
	mov var23,0
	mov var24,0
	mov var25,0
	mov var26,0
	mov var27,0
	mov var28,0
	mov var29,0
	mov var30,0
	mov var31,0
	mov var32,0
	mov var33,0
	mov var34,0
	mov var35,0
	
	mov zero_var,0
	mov carry_var,0
	mov sign_var,0
	mov overflow_var,0

	ret

	checkFlag endp

;Function to Print register Command: 
	
	Print_Register_Command proc
	
	lea dx,register_cmd_print
	mov ah,9
	int 21h
	
	ret
	
	Print_Register_Command endp

;Function to print Exit Command:

	Print_Exit_Command proc
	
	lea dx,exit_cmd_print
	mov ah,9
	int 21h
	
	ret
	
	Print_Exit_Command endp	

	end main