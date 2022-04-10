.8086
LOCALS

_TEXT	segment byte public use16 'CODE'
   ;
   ;	void interrupt keyboard::isr_handler(__CPPARGS);
   ;
	assume	cs:_TEXT, ds:DGROUP, es:NOTHING
@keyboard@isr_handler$qve	proc	far
	cli

	push	ax									; Placeholders to store
	push	ax									; original ISR address popped
												; by retf instruction.

	pushf										; Save context.
	push	ax
	push	bp
	push	sp
	push	ds

	mov		bp, DGROUP
	mov		ds, bp
	mov		bp, sp

	mov		ax, word ptr [@keyboard@old_isr]	; Store original handler SEG
	mov 	word ptr [bp + 10], ax				; to pushed ax, line 12, and
	mov		ax, word ptr [@keyboard@old_isr + 2]; store original handler
	mov 	word ptr [bp + 12], ax				; OFFSET to pushed ax, line 13

	in		al, 060h
	mov		ah, al
	and		al, 07fh
	cmp		al, 058h
	ja		short @@escape

	mov		byte ptr [_key_make_break], ah

@@escape:

ifdef DEBUG_KEYB
	call	debug_keyb_isr
endif

	pop		ds									; Restore context.
	pop		sp
	pop		bp
	pop		ax
	popf

	retf										; retf pop the placeholders
												; from the stack and do a far
												; jump to original ISR.
@keyboard@isr_handler$qve	endp

ifdef DEBUG_KEYB

debug_keyb_isr	proc	far
	push	ax
	push	bx
	push	cx
	push	ds
	push	es
	push	si
	push	di

	mov		bl, al
	and		al, 80h
	jnz		@@key_break
	mov		al, 02h
	jmp		@@key_make

@@key_break:
	mov		al, 01h

@@key_make:
	cld

	les     di, dword ptr [_keys_pressed]
	xor		bh, bh
	and		bl, 7fh
	add     di, bx
	stosb

	lds     si, dword ptr [_keys_pressed]
	mov		bx, 0b800h
	mov		es, bx
	mov		di, 80 * 2 * 4

	mov		cx, 80h
	mov		ah, 07h
@@show_keys:
	lodsb
	stosw
	loopnz	@@show_keys

	pop     di
	pop		si
	pop		es
	pop		ds
	pop		cx
	pop		bx
	pop		ax
debug_keyb_isr	endp

endif

_TEXT	ends

DGROUP	group	_DATA, _BSS

_DATA	segment word public use16 'DATA'
	extrn @keyboard@old_isr:dword
	extrn _old_key_make_break:byte
	extrn _key_make_break:byte

ifdef DEBUG_KEYB
	extrn _keys_pressed:dword
endif
_DATA	ends

_BSS	segment word public use16 'BSS'
_BSS	ends

	public	@keyboard@isr_handler$qve

	end
