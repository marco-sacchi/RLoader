#include <dos.h>
#include <conio.h>
#include <string.h>

#include "vga.hpp"

void vga::set_mode(uint8_t mode) {
	union REGS regs;

	regs.h.ah = 0x00;
	regs.h.al = mode;
	int86(0x10, &regs, &regs);
}

void vga::enable_cursor(bool_t enabled) {
	union REGS regs;

	regs.h.ah = 0x12;
	regs.h.bl = 0x34;
	regs.h.al = enabled;
	int86(0x10, &regs, &regs);
}

void vga::set_cursor_type(uint8_t start, uint8_t end) {
	union REGS regs;

	regs.h.ah = 0x01;
	regs.h.ch = start & 0x0f;
	regs.h.cl = end & 0x0f;
	int86(0x10, &regs, &regs);
}

void vga::set_cursor_pos(uint8_t row, uint8_t column) {
	union REGS regs;

	regs.h.ah = 0x02;
	regs.h.dh = row;
	regs.h.dl = column;
	regs.h.bh = 0x00; // page number
	int86(0x10, &regs, &regs);
}

void vga::get_cursor(vga::cursor_info_t *info) {
	union REGS regs;

	regs.h.ah = 0x03;
	regs.h.bh = 0x00; // page number
	int86(0x10, &regs, &regs);

	info->row = regs.h.dh;
	info->column = regs.h.dl;
	info->start_line = regs.h.ch;
	info->end_line = regs.h.cl;
}

uint8_t vga::encode_attrs(uint8_t back_color, uint8_t fore_color,
						  uint8_t blink) {
	return ((back_color & 0x0f) << 4) |
			(fore_color & 0x0f) |
			((blink & 0x01) << 7);
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
	regs.h.bh = 0x00; // page number
	regs.x.cx = count;

	int86(0x10, &regs, &regs);
}

void vga::write_string(const char *str, uint8_t attrs, ws_modes mode) {
	struct REGPACK regs;
	vga::cursor_info_t info;
	vga::get_cursor(&info);

	regs.r_ax = 0x1300 | mode;
	regs.r_bx = attrs; // page number in bh
	regs.r_cx = strlen(str);
	regs.r_dx = ((uint16_t)info.row << 8) + (uint16_t)info.column;
	regs.r_es = FP_SEG(str);
	regs.r_bp = FP_OFF(str);

	intr(0x10, &regs);
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

