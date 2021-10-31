#include <mem.h>
#include <string.h>
#include <dos.h>

#include "graphics.hpp"
#include "vga.hpp"
#include "math.hpp"

/** Step count for fade in/out methods. */
#define FADE_STEPS	64

/** Video memory pointer. */
static uint8_t far *vmem = (uint8_t far *)0xa0000000L;

void graphics::uint32_to_color(uint32_t value, vga::color_t *color) {
	(*color)[0] = (uint8_t)((value >> 16) & 0xff);
	(*color)[1] = (uint8_t)((value >> 8) & 0xff);
	(*color)[2] = (uint8_t)(value & 0xff);
}

void graphics::lerp_color(const vga::color_t from, const vga::color_t to,
						  float t, vga::color_t *result) {
	(*result)[0] = lerp(from[0], to[0], t);
	(*result)[1] = lerp(from[1], to[1], t);
	(*result)[2] = lerp(from[2], to[2], t);
}

void graphics::set_palette(bitmap *bmp) {
	uint32_t colors;
	vga::color_t color;
	uint32_t *palette = bmp->get_palette(&colors);

	for (int i = 0; i < (int)colors; ++i) {
		graphics::uint32_to_color(palette[i], &color);
		vga::set_color(i, color);
	}
}

void graphics::set_ega_palette(bitmap *bmp, int index_offset) {
	uint32_t colors;
	vga::color_t color;
	uint32_t *palette = bmp->get_palette(&colors);

	for (int i = 0; i < min((int)colors, 16 - index_offset); ++i) {
		graphics::uint32_to_color(palette[i], &color);
		vga::set_ega_color(min(15, max(0, i + index_offset)), color);
	}
}

void graphics::ega_fade_in(bitmap *bmp, const vga::color_t color, int duration) {
	int step_duration = duration / FADE_STEPS;

	for (int i = 0; i < FADE_STEPS; ++i) {
		graphics::lerp_ega_palette(bmp, color,
							(float)(FADE_STEPS - i - 1) / (float)FADE_STEPS);
		delay(step_duration);
	}

	graphics::lerp_ega_palette(bmp, color, 0.0f);
}

void graphics::ega_fade_out(bitmap *bmp, const vga::color_t color, int duration) {
	int step_duration = duration / FADE_STEPS;

	for (int i = 0; i < FADE_STEPS; ++i) {
		graphics::lerp_ega_palette(bmp, color, (float)i / (float)FADE_STEPS);
		delay(step_duration);
	}

	graphics::lerp_ega_palette(bmp, color, 1.0f);
}

void graphics::lerp_ega_palette(bitmap *bmp, const vga::color_t to_color, float t) {
	uint32_t colors;
	vga::color_t color;
	uint32_t *palette = bmp->get_palette(&colors);
	int index_offset = 0;
	for (int i = 0; i < min((int)colors, 16 - index_offset); ++i) {
		graphics::uint32_to_color(palette[i], &color);
		graphics::lerp_color(color, to_color, t, &color);
		vga::set_ega_color(min(15, max(0, i + index_offset)), color);
	}
}

void graphics::show_palette() {
	asm {
		push es

		les di, [vmem]

		mov dx, VGA_SEQ_ADDR
		mov ax, 0x0002

	}
	color_loop:
	asm {
		out dx, ax

		push ax
		mov al, 0xff
		stosb
		pop ax

		inc ah
		cmp ah, 0x0f
		jle color_loop

		pop es
	}
}

void graphics::draw(bitmap *bmp, uint16_t x, uint16_t y) {
	uint8_t *image = (uint8_t *)bmp->get_image();
	uint16_t stride = bmp->get_stride();

	uint16_t width = (uint16_t)bmp->get_width();
	uint16_t height = bmp->get_height();
	uint16_t vmem_offset = y * 80 + (x >> 3);

	asm {
		cli

		push ds
		push es
		push si
		push di
		push ax
		push bx
		push cx

		cld

		mov bx, height
		mov cx, width
		shr cx, 1
		shr cx, 1
		shr cx, 1

		les di, [vmem]
		lds si, [image]
		add di, vmem_offset
	}
	rows_loop:
	asm {
		push si

		mov dx, VGA_SEQ_ADDR
		mov ax, 0x0102
	}
	plane_loop:
	asm {
		out dx, ax

		push cx
		push di
		rep movsb
		pop di
		pop cx

		shl ah, 1
		cmp ah, 0x08
		jle plane_loop

		pop si

		add si, stride
		add di, 80
		dec bx
		cmp bx, 0
		jne rows_loop

		pop cx
		pop bx
		pop ax
		pop di
		pop si
		pop es
		pop ds

		sti
	}
}

void graphics::draw(ansi_picture *pic, uint8_t row, uint8_t column) {
	const int8_t *data = (int8_t *)pic->get_data();
    const vga::state_t *vga_state = vga::get_current_state();
	uint16_t stride = pic->get_stride();
	uint16_t width = pic->get_width(), height = pic->get_height();

	// Check if picture bottom-right character will be drawn on
	// bottom-right screen character position and trigger scroll.
	bool_t scroll = (row + height) == vga_state->rows &&
					(column + width) == vga_state->columns;

	if (scroll)
		--height;

	for (uint16_t j = 0; j < height; ++j, data += stride) {
		vga::set_cursor_pos(row + j, column);
		vga::write_string(data, width, 0,
						  vga::USE_STR_NO_UPDATE_CURSOR);
	}

	// To avoid scroll draw all rows but the last that will be rendered
	// exluding the last charater and drawing it using write_char that
	// do not trigger scrolling.
	if (scroll) {
		vga::set_cursor_pos(row + height, column);
		vga::write_string(data, width - 1, 0,
						  vga::USE_STR_UPDATE_CURSOR);
		vga::write_char(*(data + stride - 2), *(data + stride - 1), 1);
	}
}

void graphics::draw_filled_rect(vga::text_rect_t *rect, uint8_t attrs) {
	vga::scroll_page_up(rect->bottom - rect->top + 1, attrs & 0xf0, rect);
}

void graphics::draw_frame(vga::text_rect_t *rect, uint8_t attrs) {
	uint8_t width = rect->right - rect->left + 1;

	vga::set_cursor_pos(rect->top, rect->left);
	vga::write_char(0xc4, attrs, width);
	vga::write_char(0xda, attrs, 1);

	vga::set_cursor_pos(rect->top, rect->right);
	vga::write_char(0xbf, attrs, 1);

	for (int i = rect->top + 1; i < rect->bottom; ++i) {
		vga::set_cursor_pos(i, rect->left);
		vga::write_char(0xb3, attrs, 1);

		vga::set_cursor_pos(i, rect->right);
		vga::write_char(0xb3, attrs, 1);
	}

	vga::set_cursor_pos(rect->bottom, rect->left);
	vga::write_char(0xc4, attrs, width);
	vga::write_char(0xc0, attrs, 1);

	vga::set_cursor_pos(rect->bottom, rect->right);
	vga::write_char(0xd9, attrs, 1);
}

void graphics::draw_filled_frame(vga::text_rect_t *rect, uint8_t attrs) {
	graphics::draw_filled_rect(rect, attrs);
	graphics::draw_frame(rect, attrs);
}

void graphics::draw_outline(vga::text_rect_t *rect, uint8_t color) {
	int width = (rect->right - rect->left + 1) << 3;
	int height = (rect->bottom - rect->top + 1) << 4;
	int x = rect->left << 3, y = (int)rect->top << 4;

	graphics::draw_horiz_line(x, y - 1, width, color);
	graphics::draw_horiz_line(x, y + height, width, color);

	graphics::draw_vert_block(x - 1, y, height, color, 0x01);
	graphics::draw_vert_block(x + width, y, height, color, 0x80);
}

void graphics::draw_focus_outline(vga::text_rect_t *rect, uint8_t color) {
	int width = (rect->right - rect->left + 1) << 3;
	int height = (rect->bottom - rect->top + 1) << 4;
	int x = rect->left << 3, y = (int)rect->top << 4;

	graphics::draw_horiz_line(x, y - 2, width, color);
	graphics::draw_horiz_line(x, y - 1, width, color);
	graphics::draw_horiz_line(x, y + height, width, color);
	graphics::draw_horiz_line(x, y + height + 1, width, color);

	graphics::draw_vert_block(x - 1, y - 1, 1, color, 0x01);
	graphics::draw_vert_block(x - 1, y, height, color, 0x03);
	graphics::draw_vert_block(x - 1, y + height, 1, color, 0x01);

	graphics::draw_vert_block(x + width, y - 1, 1, color, 0x80);
	graphics::draw_vert_block(x + width, y, height, color, 0xc0);
	graphics::draw_vert_block(x + width, y + height, 1, color, 0x80);
}

void graphics::draw_scroll_bar(vga::text_rect_t *rect, uint8_t attrs,
							   int count, int index) {
	int height = rect->bottom - rect->top + 1;
	uint8_t scroll_column = rect->right + 1;
	if (count == 0) {
		for (int i = 0; i < height; ++i) {
			vga::set_cursor_pos(rect->top + i, scroll_column);
			vga::write_char(0xdb, attrs, 1);
		}

		return;
	}

	int thumb_size = min(height, count);
	thumb_size = (int)((long)thumb_size * (long)height / count);

	int thumb_start = (int)((long)index * (long)height / count);
	int thumb_end = thumb_start + thumb_size;

	for (int i = 0; i < height; ++i) {
		if (i == thumb_start) {
			while (i <= thumb_end && i < height) {
				vga::set_cursor_pos(rect->top + i++, scroll_column);
				vga::write_char(0xdb, attrs, 1);
			}

			if (i == height)
				break;
		}

		vga::set_cursor_pos(rect->top + i, scroll_column);
		vga::write_char(0xb1, attrs, 1);
	}
}

void graphics::bold_effect(uint8_t row, uint8_t column, uint8_t chars) {
	vga::set_read_mode(vga::PER_PLANE);
	vga::set_read_map_reg(0x01);

	vga::set_map_mask_reg(0x01);

	uint16_t vmem_offset = (row << 4) * 80 + column;
	asm {
		cli
		push es
		push di
		push cx
		push ax

		les di, [vmem]
		add di, vmem_offset
		mov cl, 16				/* characters height in pixels */
	}
	bold_loop:
	asm {
		push di
		mov ch, chars
	}
	bold_line_loop:
	asm {
		/* OR shifted 8-pixels readed to fake bold. */
		mov al, [es:di]
		mov ah, al
		shr ah, 1
		or  al, ah
		mov [es:di], al

		inc di
		dec ch
		cmp ch, 0
		jne bold_line_loop:

		pop di

		add di, 80
		dec cl
		cmp cl, 0
		jne bold_loop:

		pop ax
		pop cx
		pop di
		pop es
		sti
	}
}

void graphics::italic_effect(uint8_t row, uint8_t column, uint8_t chars) {
	vga::set_read_mode(vga::PER_PLANE);
	vga::set_read_map_reg(0x01);

	vga::set_map_mask_reg(0x01);

	uint16_t vmem_offset = (row << 4) * 80 + column;
	asm {
		cli
		push es
		push di
		push cx
		push bx
		push ax

		les di, [vmem]
		add di, vmem_offset
		mov cl, 16				/* characters height in pixels */
		mov bh, 16				/* italic right shift */
	}
	italic_loop:
	asm {
		push di
		push cx
		mov ch, chars
		mov cl, bh
		shr cl, 1
		shr cl, 1
		xor ax, ax
	}
	italic_line_loop:
	asm {
		/* shift right saving on ah the bits to render on next 8-pixels. */
		mov al, [es:di]
		mov bl, al
		shr ax, cl
		mov [es:di], al
		mov ah, bl

		inc di
		dec ch
		cmp ch, 0
		jne italic_line_loop:

		xor al, al
		shr ax, cl
		or al, [es:di]
		mov [es:di], al

		pop cx
		pop di

		add di, 80

		dec bh
		dec cl
		cmp cl, 0
		jne italic_loop:

		pop ax
		pop bx
		pop cx
		pop di
		pop es
		sti
	}
}

void graphics::draw_horiz_line(int x, int y, int length, uint8_t color) {
	graphics::or_horiz_line(x, y, length, color ^ 0xff, 0x00);
	graphics::or_horiz_line(x, y, length, color, 0xff);
}

void graphics::draw_vert_block(int x, int y, int length, uint8_t mask, uint8_t bits) {
	graphics::or_vert_block(x, y, length, mask ^ 0xff, 0x00);
	graphics::or_vert_block(x, y, length, mask, bits);
}

void graphics::or_horiz_line(int x, int y, int length, uint8_t mask, uint8_t bits) {
	uint16_t vmem_offset = y * 80 + (x >> 3);
	length >>= 3;

	vga::set_map_mask_reg(mask);
	asm {
		cli

		push	es
		push	di

		cld

		mov		cx, length
		mov		al, bits

		les		di, [vmem]
		add		di, vmem_offset

		rep		stosb

		pop		di
		pop		es
	}
}

void graphics::or_vert_block(int x, int y, int length, uint8_t mask, uint8_t bits) {
	vga::set_map_mask_reg(mask);

	uint16_t vmem_offset = y * 80 + (x >> 3);

	asm {
		cli

		push	es
		push	di

		cld

		mov		cx, length
		mov		al, bits

		les		di, [vmem]
		add		di, vmem_offset
	}
	lines_loop:
	asm {
		stosb
		add		di, 79
		loop	lines_loop

		pop		di
		pop		es
	}
}
