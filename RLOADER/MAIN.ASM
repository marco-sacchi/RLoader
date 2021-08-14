.8086
LOCALS

CR				equ	0dh					; Carriage return.
LF				equ	0ah					; Line feed.
DOS_INT			equ	21h					; DOS Functions interrupt.

program segment use16 byte public 'CODE'
		assume cs:program, ds:program, es:program, ss:program
		org 0

start:
	mov		ax, cs

	mov		ss, ax						; Relocate stack to make it available
	lea		sp, stack_end - 2			; after memory release.

	lea		bx, PSP_seg					; Store program segment prefix in
	mov		word ptr [cs:bx], ds		; PSP_seg to release unused memory.

	lea		bx, envblock_seg			; Store environment block segment
	mov		si, word ptr [ds:2ch]		; in envblock_seg to search for
	mov		word ptr [cs:bx], si		; COMSPEC environment variable.

	mov		es, ax

	mov		si, 80h						; Copy command tail to forward to
	lea		di, args					; ui executable.
	call	_cmd_tail_copy

	mov		ds, ax

	call	_init_interpreter
	cmp		ax, 1
	je		main_comspec_found

	lea		dx, err_comspec				; Display error message when
	call	_print_line					; COMSPEC environemnt variabile
										; is not defined.
	mov		al, 1
	call	_exit

main_comspec_found:
	lea		bx, param_block
	mov		word ptr [bx + 4], ds		; Set segments of param_block to ds.
	mov		word ptr [bx + 8], ds
	mov		word ptr [bx + 12], ds

	call	_release_memory				; Release the rest of segment.

	lea		si, memfree					; Display free memory info only when
	inc		di							; /batchmemfree flag is specified.
	call	_str_search
	jc		main_loop

	lea		si, cmd_memfree
	call	_shell

	lea		si, cmd_pause
	call	_shell

main_loop:
	lea		si, ui						; Launch user interface.
	lea		di, args					; Forward arguments.
	call	_exec
	cmp		ax, 0
	je		main_ui_launch_ok

	call	_delete_temp_files			; Delete temporary files.

	lea		dx, err_run_ui				; Display error message on
	call	_print_line					; user interface launch.

	mov		al, 2
	call	_exit
	
main_ui_launch_ok:
	lea 	dx, temp_batch				; Check for existence of temporary
	call	_file_exists				; batch file.

	cmp		al, 0
	jne		main_batch_launch

	xor		al, al						; If do not exists, there is no
	call	_exit						; program to be launched and loop
										; must end.
main_batch_launch:
	lea		si, cmd_batch				; Run batch testing it for error.
	call	_shell
	cmp		ax, 0
	je		main_batch_launch_ok

	lea		dx, err_run_batch			; Display error on batch lauch fail.
	call	_print_line

	mov		al, 3
	call	_exit

main_batch_launch_ok:
	lea		bx, first_run				; Check if first_run flag must
	cmp		byte ptr [bx], 0			; be cleared.
	je		main_loop					; If already cleared loop.
	
	mov		byte ptr [bx], 0			; Clear the flag.

	cld
	lea		si, args					; Add /nosplash to arguments forwarded
	mov		cx, si						; to ui executable when is not the
	lodsb								; first run.

	xor		ah, ah						; Append no_splash string.
	add		si, ax
	xchg	di, si
	lea		si, no_splash
	call	_str_copy

	cld
	xor		al, al						; Advance di until string terminator
main_args_end_search:					; is found.
	scasb
	jne main_args_end_search

	dec		di							; Append carriage return...
	mov		al, CR
	stosb
	xor		al, al						; ... and string terminator.
	stosb

	mov		si, cx						; Update characters count on first
	sub		di, cx						; byte of args.
	sub		di, 3
	mov		ax, di
	xchg	si, di
	stosb

	jmp		main_loop

	xor		al, al
	call	_exit

; Initialize interpreter string searching for COMSPEC environmental variable.
;
; returns:
; ax		1 if COMSPEC is found, 0 otherwise.
_init_interpreter proc near
	push	bx
	push	si
	push	di
	push	es
	push	ds

	mov		bx, word ptr [envblock_seg]	; Put environment block segment in es.
	mov		es, bx
	xor		di, di

	lea		si, comspec

@@search_loop:
	call	_str_search
	jnc		@@found

	mov		di, ax

	xor		ax, ax
	cmp		byte ptr [es:di], al
	je		@@not_found
	
	jmp		@@search_loop

@@found:
	add		di, comspec_len
	lea		si, interpreter
	xchg	di, si

	mov		ax, ds
	mov		es, ax
	mov		ds, bx

	call	_str_copy
	mov		ax, 1
	
@@not_found:
	pop		es
	pop		ds
	pop		di
	pop		si
	pop		bx
	ret
_init_interpreter endp

; Delete temporary files that may be left on disk.
;
_delete_temp_files proc near
	push	ax
	push	dx
	push	si
	
	lea 	dx, temp_batch
	call	_file_exists

	cmp		al, 0
	je		@@files_no_batch
	
	mov		si, dx
	call	_file_unlink

@@files_no_batch:
	lea 	dx, temp_cycles
	call	_file_exists

	cmp		al, 0
	je		@@files_no_cycles
	
	mov		si, dx
	call	_file_unlink

@@files_no_cycles:
	pop		si
	pop 	dx
	pop		ax
	ret
_delete_temp_files endp

; Program exit with return code.
;
; on entry:
; al		return code
_exit proc near
	push	ax
	
	mov		ah, 4ch
	int 	DOS_INT

	pop		ax
	ret
_exit endp

; Print string on stdout.
;
; on entry:
; ds:dx		string
_print_line proc near
	push	ax

	mov		ah, 09h
	int 	DOS_INT
	
	pop		ax
	ret
_print_line endp

; Get current drive index.
;
; returns:
; al		drive index (0 = A, 1 = B, etc.)
_get_curr_drive proc near
	push	bp
	mov		bp, sp
	push	ax
	
	mov		ah, 19h
	int 	DOS_INT

	mov 	byte ptr [bp - 2], al		; copy al to pushed ax on the stack
										; to preserve ah
	pop		ax
	pop		bp
	ret
_get_curr_drive endp

; Get current directory.
;
; on entry:
; ds:si		buffer to receive current directory.
_get_curr_dir proc near
	push	ax
	push	dx
	
	mov		ah, 47h
	mov		dl, 0
	int 	DOS_INT

	pop		dx
	pop		ax
	ret
_get_curr_dir endp

; Build fullpath prepending current drive and working dir to given filename.
;
; on entry:
; ds:si		filename.
; es:di		buffer to store fullpath.
_build_fullpath proc near
	push	ax
	push	si
	push	di

	call	_get_curr_drive 			; Build full path...
	add		al, 'A'
	stosb
	mov		al, ':'
	stosb
	mov		al, '\'
	stosb								; [drive]:\

	xchg	si, di						; Write curr dir to ds:si
	call	_get_curr_dir
	xchg	si, di

	mov		al, '\'
	call	_str_append_char			; [drive]:\[currdir]\

	call	_str_append					; Append filename in ds:di to create
										; full path of file to execute.
	pop		di
	pop		si
	pop		ax
	ret
_build_fullpath endp

; Check if file exists.
;
; on entry:
; ds:dx		filename to test.
; returns:
; al		1 if file exists, 0 otherwise.
_file_exists proc near
	push	bp
	mov		bp, sp

	push	ax
	push	cx

	mov		ah, 4eh
	xor		cx, cx
	int 	DOS_INT

	mov		al, 0
	jc		@@not_found

	mov		al, 1

@@not_found:
	pop		cx
	
	mov 	byte ptr [bp - 2], al		; copy al to pushed ax on the stack
										; to preserve ah
	pop		ax
	pop		bp
	ret
_file_exists endp

; Delete file (unlink).
;
; on entry:
; ds:si		filename to delete.
_file_unlink proc near
	push	di
	push	si
	push	ax
	push	dx

	lea		di, path_buffer
	call	_build_fullpath

	mov		ah, 41h
	mov		dx, di
	int 	DOS_INT

	pop		dx
	pop		ax
	pop		si
	pop		di
	ret
_file_unlink endp

; Release unused memory.
;
_release_memory proc near
	push	es
	push	ax
	push	bx

	mov		bx, word ptr [PSP_seg]		; Put PSP segment in es.
	mov		es, bx

	mov		ax, ds						; Compute paragraphs of memory to
	sub		ax, bx						; be resident from PSP segment to
										; resident_end label and free the
	lea		bx, resident_end			; rest.
	shr		bx, 4
	inc		bx
	add		bx, ax

	mov		ah, 4ah
	int 	DOS_INT

	pop		bx
	pop		ax
	pop		es
	ret
_release_memory endp

; Execute a program
;
; on entry:
; ds:si		filename
; es:di		command tail
; returns:
; ax        0 on success, DOS error code on failure
_exec proc near
	push	ds
	push	es
	push	dx
	push	cx
	push	bx

	cmp		word ptr [si + 1], '\:'		; \: reversed due to byte order.
	je		@@absolute_path

	push	di

	lea		di, path_buffer
	call	_build_fullpath
	mov		si, di

	pop		di

@@absolute_path:
	mov		cs:ss_save, ss
	mov		cs:sp_save, sp

	mov		dx, si
	lea 	bx, param_block
	mov		word ptr [bx + 2], di		; set param_block command tail offset
										; to input command tail string
	mov		ax, 4b00h
	int 	DOS_INT

	cli

	mov		ss, cs:ss_save
	mov		sp, cs:sp_save

	sti

	jc		@@exec_error
	mov		ax, 0

@@exec_error:
	pop		bx
	pop		cx
	pop		dx
	pop		es
	pop		ds

	ret
_exec endp

; Execute command on shell
;
; on entry:
; ds:si		command tail
; returns:
; ax        0 on success, DOS error code on failure
_shell proc near
	push	di
	push	si

	xchg	di, si
	lea		si, interpreter
	call	_exec

	pop		si
	pop		di
	ret
_shell endp

; Copy command tail style or PSP string.
;
; expected source string format:
; 1 byte	number of bytes following
; N bytes	string content bytes
; 0dh		carriage return character
;
; on entry:
; ds:si		source string
; es:di		destination string
_cmd_tail_copy proc near
	push	ax
	push	cx
	push	si
	push	di

	cld
	
	lodsb
	stosb

	mov		cl, al
	xor		ch, ch

	rep	movsb

	mov		al, CR
	stosb
	xor		al, al
	stosb

	pop		di
	pop		si
	pop		cx
	pop		ax
	ret
_cmd_tail_copy endp

; ASCIIZ string copy
;
; on entry:
; ds:si		source string
; es:di		destination string
_str_copy proc near
	push	ax
	push	si
	push	di

	cld
@@loop:
	lodsb
	stosb
	cmp		al, 0
	jne		@@loop

	pop		di
	pop		si
	pop		ax
	ret
_str_copy endp

; Append ASCIIZ string to another one.
;
; on entry:
; ds:si		string to be appended
; es:di		destination string (must have enough room for string to append)
_str_append proc near
	push	di
	push	ax

	cld									; Search for string terminator.
	xor		al, al
@@end_search:
	scasb
	jne		@@end_search

	dec		di							; Decrement auto-incremented di and
	call	_str_copy					; append the string in es:di.

	pop		ax
	pop		di
	ret
_str_append endp

; Append characted to ASCIIZ string.
;
; on entry:
; al		characted to be appended
; es:di		destination string (must have enough room for another char)
_str_append_char proc near
	push	di
	push	ax

	cld									; Search for string terminator.
	xor		al, al
@@char_search:
	scasb
	jne		@@char_search

	pop		ax
	push	ax

	dec		di							; Decrement auto-incremented di and
	stosb								; append character and terminator.
	xor		al, al
	stosb

	pop		ax
	pop		di
	ret
_str_append_char endp

; Search ASCIIZ string inside another one.
;
; on entry:
; ds:si		string to search for
; es:di		string in which search
; returns:
; ax		di index at which search string starts if CF is reset,
;			es:di index at which the scan has arrived if CF is set.
_str_search proc near
	push	bx
	push	si
	push	di

	cld
	lodsb
	mov		ah, al						; Put in AH the first character
	mov		bx, si						; to search. And save the second
										; character index.
@@start_loop:
	cmp		byte ptr [es:di], 0
	je		@@not_found_inc
	scasb
	jne		@@start_loop

@@match_loop:
	lodsb
	scasb
	je		@@match_loop

	cmp		al, 0						; Check if the string to search for
	je		@@found			; is terminated; string is found.

	cmp		byte ptr [es:di - 1], 0		; Check if the string into which
	je		@@not_found		; search is terminated.

	sub		si, bx						; Move back DI to the next character
	sub		di, si						; from which the comparison began.

	mov		si, bx						; Restore SI to second char index.
	mov		al, ah						; Restore the first char to search.
	jmp		@@start_loop

@@not_found_inc:
	inc		di

@@not_found:
	mov		ax, di

	stc
	jmp		@@end

@@found:
	sub		si, bx
	sub		di, si
	mov		ax, di

	clc

@@end:
	pop		di
	pop		si
	pop		bx
	ret
_str_search endp

	stack_buffer	db	64 dup (?)
stack_end:
	args			db	128 dup (?)
	path_buffer		db	128 dup (?)

	comspec			db	'COMSPEC=', 0
	comspec_len		equ	$ - comspec - 1
	interpreter		db	'C:\COMMAND.COM', (32 - ($ - interpreter)) dup (0)
	ui				db	'UI.EXE', 0
	temp_batch		db	'RLOADRUN.BAT', 0
	temp_cycles		db	'CYCLES.$$$', 0
	err_comspec		db	'COMSPEC environment variable not set.', CR, LF, '$'
	err_run_ui		db	'Error running: ui.exe', CR, LF, '$'
	err_run_batch	db	'Error running: rloadrun.bat', CR, LF, '$'

	first_run		db	1
	memfree			db	' /batchmemfree', 0
	no_splash		db	' /nosplash', 0
	cmd_memfree     db	10, ' /C MEM /F', CR
	cmd_pause	    db	9, ' /C PAUSE', CR
	cmd_batch		db	16, ' /C RLOADRUN.BAT', CR

	param_block		dw	0				; Default environment.
					dw	0				; Offset of command tail.
					dw	0				; Segment of command tail.
					dw	005ch			; Offset of first FCB, 5ch = use mine
					dw	0				; Segment of first FCB.
					dw	006ch			; Offset of second FCB, 6ch = use mine
					dw	0				; Segment of second FCB.

	PSP_seg			dw	?				; PSP segment.
	envblock_seg	dw	?				; Segment of environemnt block.
	ss_save			dw	?				; SS register value saved before EXEC.
	sp_save			dw	?				; SP register value saved before EXEC.

resident_end:

program ends

	end
