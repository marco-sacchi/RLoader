#include <conio.h>
#include <stdio.h>
#include <dos.h>

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

bool_t tests(bool_t graphics) {
	if (graphics == TRUE)
		vga::set_mode(0x12);

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
