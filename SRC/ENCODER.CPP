/** Simulate low-end machine. */
//#define SLOW_MACHINE

#ifdef SLOW_MACHINE
#include <dos.h>
#endif

#include "encoder.hpp"

#define NEW_ENCODER 1
//#define OLD_ENCODER 1

bitmap_encoder::bitmap_encoder(bitmap *bmp, uint8_t color_offset) {
	m_bitmap = bmp;
	m_image = (uint8_t *)bmp->get_image();
	m_current_ptr = m_image;
	m_remaining_rows = (int)bmp->get_height();
	m_color_offset = color_offset;
	m_color_offset |= m_color_offset << 4;
	m_plane_stride = (uint16_t)m_bitmap->get_width() >> 3;
}

bitmap_encoder::~bitmap_encoder() {
	m_bitmap = NULL;
	m_image = NULL;
}

bool_t bitmap_encoder::task() {
	if (m_bitmap == NULL || m_image == NULL || m_remaining_rows == 0)
		return FALSE;

	this->planar_4bits(m_current_ptr);

	m_current_ptr += (uint16_t)m_bitmap->get_stride();
	--m_remaining_rows;

#ifdef SLOW_MACHINE
	delay(10);
#endif

	if (m_remaining_rows == 0)
		return TRUE;

	return FALSE;
}

void bitmap_encoder::planar_4bits(const uint8_t *row) {
#if NEW_ENCODER
	uint16_t plane_stride = m_plane_stride;
	uint8_t *encoded = m_encode_buffer;
	uint8_t col_offset = (uint8_t)m_color_offset;

	asm {
		cli
		cld

		push	es
		push	ds

		lds		si, [row]
		les		di, [encoded]

		mov		cx, plane_stride		// compute end offset
		add		cx, di

		mov		ah, col_offset
	}
	next_packed:
	asm {
		push	cx

		xor		bx, bx					// bl = plane0, bh = plane1 work bytes
		xor		dx, dx					// dl = plane2, dh = plane3 work bytes
		xor		cl, cl					// zero-based plane number
		mov		ch, 6					// destination bit shift for packing
	}
	next_byte:
	asm {
		lodsb							// al <- ds:si, two 4-bits pixels
		add		al, ah					// add color offset to the pixels

		mov		cl, 3                   // packing shifts count

		push	ax						// plane 0
		and		al, 0x11				// mask two pixels to get plane 0
		mov		ah, al					// copy masked value
		shr		ah, cl                  // shift copy to pack with original
		and		al, 0x0f				// mask pixel in the upper 4-bits
		or		al, ah					// merge copy and original bits

		xchg	cl, ch					// shift left on proper position
		shl		al, cl
		xchg	cl, ch

		or		bl, al					// OR to plane 0 working byte
		pop		ax

		push	ax						// plane 1
		and		al, 0x22				// mask two pixels to get plane 1
		mov		ah, al					// copy masked value
		shr		ah, cl					// shift copy to pack with original
		and		al, 0x0f				// mask pixel in the upper 4-bits
		or		al, ah					// merge copy and original bits
		shr		al, 1					// shift to 000000xxb position

		xchg	cl, ch					// shift left on proper position
		shl		al,	cl
		xchg	cl, ch

		or		bh, al					// OR to plane 1 working byte
		pop		ax

		push	ax						// plane 2
		and		al, 0x44				// mask two pixels to get plane 2
		mov		ah, al					// copy masked value
		shr		ah, cl                  // shift copy to pack with original
		and		al, 0x0f				// mask pixel in the upper 4-bits
		or		al, ah					// merge copy and original bits
		dec		cl						// plane 2
		shr		al, cl					// shift to 000000xxb position

		xchg	cl, ch					// shift left on proper position
		shl		al, cl
		xchg	cl, ch

		or		dl, al					// OR to plane 2 working byte
		pop		ax

		push	ax						// plane 3
		and		al, 0x88				// mask two pixels to get plane 3
		mov		ah, al					// copy masked value
		inc		cl						// shift copy to pack with original
		shr		ah, cl
		and		al, 0x0f				// mask pixel in the upper 4-bits
		or		al, ah					// merge copy and original bits
		shr		al, cl					// shift to 000000xxb position

		xchg	cl, ch					// shift left on proper position
		shl		al, cl
		xchg	cl, ch

		or		dh, al					// OR to plane 3 working byte
		pop		ax

		sub		ch, 2					// update shift value to next two bits
		jns		next_byte				// no sign, store on work bytes

		mov		al, bl					// write packed bits for plane 0
		stosb

		push	di						// store di for plane 0

		mov		cx, plane_stride
		dec		cx

		add		di, cx					// di is already incremented by stosb
		mov		al, bh					// write packed bits for plane 1
		stosb

		add		di, cx					// di is already incremented by stosb
		mov		al, dl					// write packed bits for plane 2
		stosb

		add		di, cx					// di is already incremented by stosb
		mov		al, dh					// write packed bits for plane 3
		stosb

		pop		di						// restore di for plane 0 next byte

		pop		cx						// restore plane stride

		cmp		di, cx
		jge		done					// di == plane stride, done.

		jmp		next_packed				// ready to pack next 4 bytes
	}
	done:
	asm {
		lds		si, [encoded]			// reverse source and destination
		les		di, [row]

		mov		cx, plane_stride		// compute buffer length
		shl		cx, 1
		shl		cx, 1

		mov		bx, cx
		and		bx, 1
		jz		even					// check for odd length

		movsb							// move one byte
	}
	even:
	asm {
		shr		cx, 1
		rep		movsw					// move a word at a time

		pop		ds
		pop		es

		sti
	}

#elif OLD_ENCODER

	uint16_t width = (uint16_t)m_info.width;
	uint8_t *encoded = m_encode_buffer;
	uint8_t col_offset = (uint8_t)color_offset;

	asm {
		push ax
		push bx
		push cx
		push dx
		push si
		push di
		push ds
		push es

		lds si, [row]
		les di, [encoded]

		cld

		mov dh, col_offset
		mov dl, dh
		mov cl, 4
		shl dl, cl
		or dh, dl

		xor dl, dl					/* plane index */
	}
	plane_loop:
	asm {
		mov cx, width
		dec cx

		push si
	}
	row_loop:
	asm {
		mov bl, 4					/* shift to select pixel, alternate 0-4 */
		mov bh, 8					/* 8 pixels block counter */
		xor ax, ax

		push cx
	}
	block_loop_load:
	asm {
		lodsb
		add al, dh
		mov ch, al
	}
	block_loop_noload:
	asm {
		mov al, ch

		mov cl, bl
		shr al, cl
		mov cl, dl
		shr al, cl

		and al, 1
		mov cl, bh
		dec cl
		shl al, cl

		or ah, al

		xor bl, 4
		dec bh
		cmp bh, 0
		je block_done
		cmp bl, 0
		je block_loop_noload
		jmp block_loop_load
	}
	block_done:
	asm {
		mov al, ah
		stosb

		pop cx

		sub cx, 8
		jnc row_loop

		pop si

		inc dl
		cmp dl, 4
		jl plane_loop

		pop es
		pop ds
		pop di
		pop si
		pop dx
		pop cx
		pop bx
		pop ax
	}

	memcpy((void *)row, m_encode_buffer, (size_t)m_stride);

#else

	uint8_t *pixel;
	uint8_t *encoded = m_encode_buffer;
	uint8_t block_size = (8 * m_info.bpp) >> 3;
	uint8_t pixel_shift = 4;

	pixel = (uint8_t *)row;
	for (int32_t x = 0; x < m_stride; ++x)
		*pixel++ = (uint8_t)(((*pixel & 0x0f) + color_offset) |
			(*pixel & 0xf0) + (color_offset << 4));

	for (uint16_t plane = 0; plane < 4; ++plane) {
		pixel = (uint8_t *)row;
		for (int32_t i = 0; i < m_info.width; i += 8) {
			*encoded = 0;
			for (uint16_t j = 0; j < 8; ++j, pixel_shift ^= 4)
				*encoded |=
					(((pixel[j >> 1]      /* select byte */
					>> pixel_shift        /* select 4-bit pixel */
					) >> plane) & 1)      /* get pixel bit by plane */
					<< (7 - j);           /* shift to set correct bit */

			++encoded;
			pixel += block_size;
		}
	}

	memcpy((void *)row, m_encode_buffer, (size_t)m_stride);
#endif
}
