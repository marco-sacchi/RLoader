#include <malloc.h>
#include <mem.h>
#include <stdio.h>

#include "bitmap.hpp"

#define NEW_DECODER

bitmap::bitmap() {
	memset(&m_info, 0, sizeof(bitmap::info_header_t));
	m_error = BMP_ERR_NONE;
	m_stride = 0;
	m_palette = NULL;
	m_image = NULL;
}

bitmap::~bitmap() {
	this->unload();
}

int bitmap::get_last_error() {
	return m_error;
}

uint32_t bitmap::get_width() {
	return m_info.width;
}

uint32_t bitmap::get_height() {
	return m_info.height;
}

uint32_t *bitmap::get_palette(uint32_t *colors) {
	*colors = m_info.colors;
	return m_palette;
}

void *bitmap::get_image() {
	return m_image;
}

uint32_t bitmap::get_stride() {
	return m_stride;
}

bool_t bitmap::load(const char *filename, int color_offset) {
	this->unload();

	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		m_error = BMP_ERR_NOTFOUND;
		return FALSE;
	}

	bitmap::file_header_t file_header;
	fread(&file_header, sizeof(bitmap::file_header_t), 1, fp);

	if (file_header.type != BMP_TYPE) {
		fclose(fp);
		m_error = BMP_ERR_INVALID;
		return FALSE;
	}

	fread(&(m_info.size), 4, 1, fp);
	fread(((uint8_t *)&m_info + 4), (size_t)m_info.size - 4, 1, fp);

	if (m_info.colors > 0) {
		m_palette = (uint32_t *)malloc((size_t)m_info.colors * 4);
		if (m_palette == NULL) {
			fclose(fp);
			this->unload();
			m_error = BMP_ERR_NOMEM;
			return FALSE;
		}

		fread(m_palette, (size_t)m_info.colors * 4, 1, fp);
	}

	fseek(fp, file_header.offset, SEEK_SET);
	m_stride = (((m_info.bpp * m_info.width) >> 3) + 3) & ~0x03;

	m_image = malloc((size_t)(m_stride * m_info.height));
	if (m_image == NULL) {
		fclose(fp);
		this->unload();
		m_error = BMP_ERR_NOMEM;
		return FALSE;
	}

	uint32_t i;
	uint8_t *image_ptr = (uint8_t *)m_image;
	for (i = 0; i < m_info.height - 1; ++i)
		image_ptr += (uint16_t)m_stride;

	for (i = 0; i < m_info.height; ++i) {
		fread(image_ptr, (size_t)m_stride, 1, fp);
		this->encode_planar_4bits(image_ptr, color_offset);
		image_ptr -= (uint16_t)m_stride;
	}

	fclose(fp);

	return TRUE;
}

void bitmap::encode_planar_4bits(const uint8_t *row, int color_offset) {
#ifdef NEW_DECODER
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
		shl dl, 4
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

void bitmap::unload() {
	if (m_palette != NULL)
		free(m_palette);

	if (m_image != NULL)
		free(m_image);

	memset(&m_info, 0x00, sizeof(bitmap::info_header_t));
	m_error = BMP_ERR_NONE;
	m_stride = 0;
	m_palette = NULL;
	m_image = NULL;
}
