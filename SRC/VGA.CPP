#include <dos.h>
#include <conio.h>
#include <string.h>

#include "vga.hpp"

/** Monochrome text-modes video memory pointer. */
static uint8_t far *text_vmem_bw = (uint8_t far *)0xb0000000L;
/** Color text-modes video memory pointer. */
static uint8_t far *text_vmem_color = (uint8_t far *)0xb8000000L;
/** Graphics-modes video memory pointer. */
static uint8_t far *gfx_vmem = (uint8_t far *)0xa0000000L;
/** Current text-mode video memory pointer. */
static uint8_t far *text_vmem = NULL;
/** Current video state. */
static vga::state_t curr_state;

void vga::init() {
	union REGS regs;
	int86(0x11, &regs, &regs);

	// Check for monochrome or color display adapter.
	if ((regs.h.al & 0x30) == 0x30)
		text_vmem = text_vmem_bw;
	else
		text_vmem = text_vmem_color;

	vga::read_state(&curr_state);
}

bool_t vga::is_monochrome() {
	return (text_vmem == text_vmem_bw) ? TRUE : FALSE;
}

void vga::set_mode(uint8_t mode) {
	union REGS regs;

	regs.h.ah = 0x00;
	regs.h.al = mode;
	int86(0x10, &regs, &regs);

	vga::read_state(&curr_state);
}

void vga::read_state(vga::state_t *state) {
	union REGS regs;

	regs.h.ah = 0x0f;
	int86(0x10, &regs, &regs);

	state->mode = regs.h.al;
	state->columns = regs.h.ah;
	state->current_page = regs.h.bh;

	if (state->mode <= 0x03 || state->mode == 0x07) {
		state->type = vga::ALPHANUMERIC;
		state->vmem = text_vmem;
	} else {
		state->type = vga::ALL_POINTS_ADDRESSABLE;
		state->vmem = gfx_vmem;
	}

	state->rows = *((uint8_t far *)0x00400084L) + 1;

	switch (state->mode) {
		case 0x04:
		case 0x05:
		case 0x0a:
			state->colors = 4;
			break;

		case 0x06:
		case 0x11:
			state->colors = 2;
			break;

		case 0x07:
		case 0x0f:
			state->colors = 1;
			break;

		default:
			state->colors = 16;
	}

	switch (state->mode) {
		case 0x00:
		case 0x02:
		case 0x05:
		case 0x07:
		case 0x0f:
			state->is_grayscale = TRUE;
			break;

		default:
			state->is_grayscale = FALSE;
	}
}

const vga::state_t *vga::get_current_state() {
	return &curr_state;
}

const uint8_t far *vga::get_vmem_pointer() {
	return curr_state.vmem;
}

void vga::enable_cursor(bool_t enabled) {
	vga::cursor_info_t info;
	vga::get_cursor(&info);

	union REGS regs;

	regs.h.ah = 0x01;
	regs.h.ch = info.start_line;
	regs.h.cl = info.end_line;

	if (enabled == FALSE)
		regs.h.ch |= 0x20;

	int86(0x10, &regs, &regs);
}

void vga::set_cursor_type(uint8_t start, uint8_t end) {
	union REGS regs;

	regs.h.ah = 0x01;
	regs.h.ch = start & 0x1f;
	regs.h.cl = end & 0x1f;
	int86(0x10, &regs, &regs);
}

void vga::set_cursor_pos(uint8_t row, uint8_t column) {
	union REGS regs;

	regs.h.ah = 0x02;
	regs.h.dh = row;
	regs.h.dl = column;
	regs.h.bh = curr_state.current_page;
	int86(0x10, &regs, &regs);
}

void vga::get_cursor(vga::cursor_info_t *info) {
	union REGS regs;

	regs.h.ah = 0x03;
	regs.h.bh = curr_state.current_page;
	int86(0x10, &regs, &regs);

	info->row = regs.h.dh;
	info->column = regs.h.dl;
	info->start_line = regs.h.ch & 0x1f;
	info->end_line = regs.h.cl & 0x1f;
}

uint8_t vga::encode_attrs(uint8_t back_color, uint8_t fore_color,
						  uint8_t blink) {
	return ((back_color & 0x0f) << 4) |
			(fore_color & 0x0f) |
			((blink & 0x01) << 7);
}

void vga::decode_attrs(uint8_t attrs, uint8_t *back_color,
					   uint8_t *fore_color, uint8_t *blink) {
	*fore_color = attrs & 0x0f;
	// TODO: track background intensity and get only 3 of 4 upper bits
	//       leaving blink at zero when backgroud intensity is enabled.
	*back_color = (attrs & 0xf0) >> 4;
	*blink = (attrs >> 7);
}

uint8_t vga::swap_attrs(uint8_t attrs) {
	return (attrs >> 4) | ((attrs & 0x0f) << 4);
}

void vga::scroll_page_up(uint8_t rows, uint8_t attrs,
						 const vga::text_rect_t *rect) {
	union REGS regs;

	regs.h.ah = 0x06;
	regs.h.al = rows;
	regs.h.bh = attrs;

	if (rect != NULL) {
		regs.h.ch = rect->top;
		regs.h.cl = rect->left;
		regs.h.dh = rect->bottom;
		regs.h.dl = rect->right;
	}

	int86(0x10, &regs, &regs);
}

void vga::scroll_page_down(uint8_t rows, uint8_t attrs,
						   const vga::text_rect_t *rect) {
	union REGS regs;

	regs.h.ah = 0x07;
	regs.h.al = rows;
	regs.h.bh = attrs;

	if (rect != NULL) {
		regs.h.ch = rect->top;
		regs.h.cl = rect->left;
		regs.h.dh = rect->bottom;
		regs.h.dl = rect->right;
	}

	int86(0x10, &regs, &regs);
}

void vga::write_char(uint8_t ch, uint8_t attrs, uint16_t count) {
	union REGS regs;

	regs.h.ah = 0x09;
	regs.h.al = ch;
	regs.h.bl = attrs;
	regs.h.bh = curr_state.current_page;
	regs.x.cx = count;

	int86(0x10, &regs, &regs);
}

void vga::write_string(const char *str, uint8_t attrs, ws_modes mode) {
	vga::write_string(str, strlen(str), attrs, mode);
}

void vga::write_string(const char *str, uint16_t length, uint8_t attrs,
					   ws_modes mode) {
	struct REGPACK regs;
	vga::cursor_info_t info;
	vga::get_cursor(&info);

	regs.r_ax = 0x1300 | mode;
	regs.r_bx = attrs | ((uint16_t)curr_state.current_page << 8);
	regs.r_cx = length;
	regs.r_dx = ((uint16_t)info.row << 8) + (uint16_t)info.column;
	regs.r_es = FP_SEG(str);
	regs.r_bp = FP_OFF(str);

	intr(0x10, &regs);
}

void vga::write_attrs(uint8_t row, uint8_t col, uint8_t new_attrs,
					  uint16_t count) {
	uint16_t start_offset = (uint16_t)row * curr_state.columns * 2 +
							(uint16_t)col * 2;

	asm {
		push es
		push di

		les di, [text_vmem]
		add di, start_offset
		inc di

		mov al, new_attrs
		mov cx, count

		cld
	}
	attrs_loop:
	asm {
		stosb
		inc di
		loop attrs_loop

		pop di
		pop es
	}
}

void vga::or_attrs(uint8_t row, uint8_t col, uint8_t or_attrs,
					uint16_t count) {
	uint16_t start_offset = (uint16_t)row * curr_state.columns * 2 +
							(uint16_t)col * 2;
	asm {
		push es
		push di
		push ds
		push si

		les di, [text_vmem]
		add di, start_offset
		inc di

		mov si, es
		mov ds, si
		mov si, di

		mov ah, or_attrs
		mov cx, count

		cld
	}
	attrs_loop:
	asm {
		lodsb
		or al, ah
		stosb

		inc di
		inc si
		loop attrs_loop

		pop si
		pop ds
		pop di
		pop es
	}
}

void vga::xor_attrs(uint8_t row, uint8_t col, uint8_t xor_attrs,
					uint16_t count) {
	uint16_t start_offset = (uint16_t)row * curr_state.columns * 2 +
							(uint16_t)col * 2;
	asm {
		push es
		push di
		push ds
		push si

		les di, [text_vmem]
		add di, start_offset
		inc di

		mov si, es
		mov ds, si
		mov si, di

		mov ah, xor_attrs
		mov cx, count

		cld
	}
	attrs_loop:
	asm {
		lodsb
		xor al, ah
		stosb

		inc di
		inc si
		loop attrs_loop

		pop si
		pop ds
		pop di
		pop es
	}
}

void vga::and_attrs(uint8_t row, uint8_t col, uint8_t and_attrs,
					uint16_t count) {
	uint16_t start_offset = (uint16_t)row * curr_state.columns * 2 +
							(uint16_t)col * 2;
	asm {
		push es
		push di
		push ds
		push si

		les di, [text_vmem]
		add di, start_offset
		inc di

		mov si, es
		mov ds, si
		mov si, di

		mov ah, and_attrs
		mov cx, count

		cld
	}
	attrs_loop:
	asm {
		lodsb
		and al, ah
		stosb

		inc di
		inc si
		loop attrs_loop

		pop si
		pop ds
		pop di
		pop es
	}
}

void vga::set_blinking(bool_t blink) {
	union REGS regs;

	regs.h.ah = 0x10;
	regs.h.al = 0x03;
	regs.h.bl = blink;

	int86(0x10, &regs, &regs);
}

void vga::set_memory_mode_reg(bool_t linear, bool_t graphics,
							  bool_t mem_limit) {
	uint8_t data = (linear << 3) | (graphics << 2) | (mem_limit << 1);
	outport(VGA_SEQ_ADDR, 0x0004 | (((uint16_t)data & 0x0f) << 8));
}

void vga::set_map_mask_reg(uint8_t planes) {
	outport(VGA_SEQ_ADDR, 0x0002 | (((uint16_t)planes & 0x0f) << 8));
}

void vga::set_enable_reg(uint8_t enable) {
	outport(VGA_GFX_ADDR, 0x0001 | (((uint16_t)enable & 0x0f) << 8));
}

void vga::set_setreset_reg(uint8_t setreset) {
	outport(VGA_GFX_ADDR, 0x0000 | (((uint16_t)setreset & 0x0f) << 8));
}

void vga::set_data_rotate_reg(uint8_t rotate_count, vga::rot_modes operation) {
	uint16_t data = ((uint16_t)((rotate_count & 0x07) |
					((operation & 0x3) << 3)) << 8);
	outport(VGA_GFX_ADDR, 0x0003 | data);
}

void vga::set_read_map_reg(uint8_t map_index) {
	uint16_t data = (uint16_t)(map_index & 0x03);
	outport(VGA_GFX_ADDR, 0x0004 | data);
}

void vga::set_bit_mask(uint8_t mask) {
	outport(VGA_GFX_ADDR, 0x0008 | ((uint16_t)mask << 8));
}

void vga::set_read_mode(vga::gfx_rmodes mode) {
	outportb(VGA_GFX_ADDR, 0x05);
	uint16_t value = inportb(VGA_GFX_DATA) & ~(1 << 3);
	value |= mode << 3;
	outport(VGA_GFX_ADDR, 0x0005 | (value << 8));
}

void vga::set_write_mode(vga::gfx_wmodes mode) {
	outportb(VGA_GFX_ADDR, 0x05);
	uint16_t value = inportb(VGA_GFX_DATA) & ~0x03;
	value |= mode;
	outport(VGA_GFX_ADDR, 0x0005 | (value << 8));
}

void vga::set_ega_colors_lookup() {
	for (uint16_t i = 0; i < 16; ++i)
		outpw(VGA_ATTR_ADDR, (i << 8) | i);
}

static ega_colors_lookup[16] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

void vga::set_ega_color(uint8_t index, const vga::color_t rgb) {
	vga::set_color(ega_colors_lookup[index], rgb);
}

void vga::set_color(uint8_t index, const vga::color_t rgb) {
	outp(VGA_DAC_WRITE, index);
	outp(VGA_DAC_DATA, rgb[0] >> 2);
	outp(VGA_DAC_DATA, rgb[1] >> 2);
	outp(VGA_DAC_DATA, rgb[2] >> 2);
}

