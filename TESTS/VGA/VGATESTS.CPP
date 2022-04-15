#include <conio.h>
#include <stdio.h>
#include <dos.h>
#include <mem.h>

#include "vga/vgatests.hpp"

#include "vga.hpp"
#include "graphics.hpp"
#include "bitmap.hpp"
#include "encoder.hpp"

/** Do graphics or text-mode tests.
 *
 * @param graphics TRUE for graphics tests, FALSE for text-mode tests.
 * @return TRUE on success, FALSE otherwise.
 */
bool_t tests(bool_t graphics);

bool_t vga_tests() {
	if (tests(FALSE) == FALSE)
		return FALSE;

	bool_t result = tests(TRUE);
	vga::set_mode(0x03);

	return result;
}

bool_t display_bitmap() {
	bitmap *bmp = new bitmap();

	if (bmp->load("../lists/games/titles/doom.bmp") == FALSE) {
		delete bmp;
		printf("Cannot allocate bitmap.\n");
		return FALSE;
	}

	bitmap_encoder *encoder = new bitmap_encoder(bmp, 2);
	if (encoder == NULL) {
		delete bmp;
		printf("Cannot allocate encoder.\n");
		return FALSE;
	}

	printf("Encoding");
	while (encoder->task() == FALSE)
		printf(".");

	graphics::set_ega_palette(bmp, 2);
	graphics::draw(bmp, 320, (480 >> 1) - (bmp->get_height() >> 1));
	delete bmp;
	bmp = NULL;

	delete encoder;
	encoder = NULL;

	return TRUE;
}

bool_t draw_font_bitmap(vga::font_info_ptr pointer, vga::font_info_t *info) {
	uint8_t far *bitmap = (uint8_t far *)info->pointer;
	int height = 0;
	switch (pointer) {
		case vga::INT_1F_VECTOR:
		case vga::INT_43_VECTOR:
			return TRUE;

		case vga::ROM_8x14_FONT:
		case vga::ROM_9x14_ALT_FONT:
			height = 14;
			break;

		case vga::ROM_8x8_2DOT_FONT:
		case vga::ROM_8x8_2DOT_FONT_TOP:
			height = 8;
			break;

		case vga::ROM_8x16_FONT:
		case vga::ROM_9x16_ALT_FONT:
			height = 16;
			break;

		default:
			return FALSE;
	}

	for (int ch = 0; ch < 10; ++ch) {
		for (int j = 0; j < height; ++j) {
			uint8_t line = *bitmap++;

			for (int i = 0; i < 8; ++i) {
				vga::set_cursor_pos(j + 2, 7 - i + ch * 8);
				vga::write_char(((line >> i) & 1) ? 0xdb : ' ', 0x07, 1);
			}
		}
	}

	return TRUE;
}

void display_font_info(vga::font_info_ptr pointer) {
	vga::font_info_t info;
	vga::get_font_info(pointer, &info);

	clrscr();
	printf("Pointer %d: %p, scanlines: %d, rows: %d\n", pointer,
		   info.pointer, info.scanlines_per_char, info.chars_rows);

	draw_font_bitmap(pointer, &info);

	getch();
}

void custom_alpha_chars() {
	const int num_chars = 8, first_char = 0xc8;

	uint8_t previous[num_chars * 16];
	uint8_t new_chars[num_chars * 16] = {
		0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,

		0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
		0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,

		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,

		0xff, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
		0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,

		0xff, 0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,

		0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
		0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xff, 0xff,

		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0xff
	};

	vga::font_info_t info;
	vga::get_font_info(vga::ROM_8x16_FONT, &info);
	memcpy(previous, (uint8_t far *)info.pointer + first_char * 16,
		   num_chars * 16);

	uint8_t chars[num_chars + 1];
	for (int i = 0; i < num_chars; ++i)
		chars[i] = i + first_char;

	chars[i] = NULL;

	clrscr();
	printf("Next characters will be modified: %s", chars);
	getch();

	vga::load_alpha_font(new_chars, 16, num_chars, first_char, 0);

	getch();

	vga::load_alpha_font(previous, 16, num_chars, first_char, 0);

	getch();
}

bool_t tests(bool_t graphics) {
	if (graphics == TRUE)
		vga::set_mode(0x12);

	display_font_info(vga::INT_1F_VECTOR);
	display_font_info(vga::INT_43_VECTOR);
	display_font_info(vga::ROM_8x14_FONT);
	display_font_info(vga::ROM_8x8_2DOT_FONT);
	display_font_info(vga::ROM_8x8_2DOT_FONT_TOP);
	display_font_info(vga::ROM_9x14_ALT_FONT);
	display_font_info(vga::ROM_8x16_FONT);
	display_font_info(vga::ROM_9x16_ALT_FONT);

	if (graphics == FALSE)
		custom_alpha_chars();

	vga::set_cursor_type(0, 15);
	vga::set_cursor_pos(3, 16);

	uint8_t attrs = vga::encode_attrs(0, 1, 0);

	vga::write_char('c', attrs, 40);

	getch();

	vga::text_rect_t rect;
	rect.top = 0;
	rect.left = 0;
	rect.bottom = 24;
	rect.right = 39;
	vga::scroll_page_up(1, attrs, &rect);

	vga::set_cursor_pos(24, 16);
	vga::write_char('a', attrs, 2);
	vga::write_string("Test string", attrs, vga::USE_ATTRS_UPDATE_CURSOR);

	getch();

	if (graphics == TRUE) {
		if (display_bitmap() == FALSE)
			return FALSE;

		getch();
	}

	rect.top = 0;
	rect.left = 40;
	rect.bottom = 39;
	rect.right = 79;
	vga::set_map_mask_reg(0x0f);

	attrs = vga::encode_attrs(0, 0, 0);

	int i = 15;
	while (i-- > 0) {
		vga::scroll_page_up(1, attrs, &rect);
		delay(10);
	}

	getch();

	i = 15;
	while (i-- > 0) {
		vga::scroll_page_up(1, attrs, &rect);
		delay(10);
	}

	getch();

	return TRUE;
}

