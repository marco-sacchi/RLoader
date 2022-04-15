#include <mem.h>

#include "tui/tui.hpp"
#include "tui/graph.hpp"
#include "tui/theme.hpp"

#include "math.hpp"

bool_t uigraph::custom_frame = FALSE;
uint8_t uigraph::previous_chars[FRAME_CHARS * 16];
int uigraph::previous_char_height = 0;

#define FRAME_TOP_LEFT		0
#define FRAME_TOP			1
#define FRAME_TOP_RIGHT		2
#define FRAME_LEFT			3
#define FRAME_RIGHT			4
#define FRAME_BOTTOM_LEFT	5
#define FRAME_BOTTOM		6
#define FRAME_BOTTOM_RIGHT	7

uint8_t uigraph::frame_chars[FRAME_CHARS] = {
	0xc9, 0xcd, 0xbb,
	0xba,       0xba,
	0xc8, 0xcd, 0xbc
};

bool_t uigraph::setup_frame_chars() {
	if (tui::text_mode == FALSE)
		return TRUE;

	vga::font_info_t info;
	vga::get_font_info(vga::ROM_8x8_2DOT_FONT, &info);
	int char_height = info.scanlines_per_char;

	switch (char_height) {
		case 8:
			// CGA
			vga::get_font_info(vga::ROM_8x8_2DOT_FONT, &info);
			break;

		case 14:
			// EGA
			vga::get_font_info(vga::ROM_8x14_FONT, &info);
			break;

		case 16:
			// VGA
			vga::get_font_info(vga::ROM_8x16_FONT, &info);
			break;

		default:
			return FALSE;
	}

	memcpy(uigraph::previous_chars,
		   (uint8_t far *)info.pointer + 0xba * char_height,
		   1 * char_height);

	memcpy(uigraph::previous_chars + char_height,
		   (uint8_t far *)info.pointer + 0xc8 * char_height,
		   (FRAME_CHARS - 1) * char_height);

	uint8_t new_chars[FRAME_CHARS * 16];
	uint8_t *char_line = new_chars;

	// Left side
	memset(char_line, 0xc0, char_height);
	char_line += char_height;

	// Right side
	memset(char_line, 0x01, char_height);
	char_line += char_height;

	// Top side
	*char_line++ = 0xff;
	*char_line++ = 0xff;
	memset(char_line, 0x00, char_height - 2);
	char_line += char_height - 2;

	// Bottom side
	memset(char_line, 0x00, char_height - 2);
	char_line += char_height - 2;
	*char_line++ = 0xff;
	*char_line++ = 0xff;

	// Top-left corner
	*char_line++ = 0xff;
	*char_line++ = 0xff;
	memset(char_line, 0xc0, char_height - 2);
	char_line += char_height - 2;

	// Top-right corner
	*char_line++ = 0xff;
	*char_line++ = 0xff;
	memset(char_line, 0x01, char_height - 2);
	char_line += char_height - 2;

	// Bottom-left corner
	memset(char_line, 0xc0, char_height - 2);
	char_line += char_height - 2;
	*char_line++ = 0xff;
	*char_line++ = 0xff;

	// Bottom-right corner
	memset(char_line, 0x01, char_height - 2);
	char_line += char_height - 2;
	*char_line++ = 0xff;
	*char_line++ = 0xff;

	vga::load_alpha_font(new_chars, char_height, 1, 0xba, 0);
	vga::load_alpha_font(new_chars + char_height, char_height,
						 FRAME_CHARS - 1, 0xc8, 0);

	uigraph::previous_char_height = char_height;
	uigraph::custom_frame = TRUE;

	uigraph::frame_chars[FRAME_TOP_LEFT] = 0xcb;
	uigraph::frame_chars[FRAME_TOP] = 0xc9;
	uigraph::frame_chars[FRAME_TOP_RIGHT] = 0xcc;
	uigraph::frame_chars[FRAME_LEFT] = 0xba;
	uigraph::frame_chars[FRAME_RIGHT] = 0xc8;
	uigraph::frame_chars[FRAME_BOTTOM_LEFT] = 0xcd;
	uigraph::frame_chars[FRAME_BOTTOM] = 0xca;
	uigraph::frame_chars[FRAME_BOTTOM_RIGHT] = 0xce;

	return TRUE;
}

void uigraph::restore_chars() {
	if (uigraph::custom_frame == FALSE)
		return;

	vga::load_alpha_font(uigraph::previous_chars,
						 uigraph::previous_char_height, 1, 0xba, 0);
	vga::load_alpha_font(uigraph::previous_chars +
							uigraph::previous_char_height,
						 uigraph::previous_char_height,
						 FRAME_CHARS - 1, 0xc8, 0);
}

void uigraph::enable_caret(bool_t enable) {
	if (tui::text_mode == TRUE)
		vga::enable_cursor(enable);
}

void uigraph::draw_frame(vga::text_rect_t *rect, uint8_t attrs) {
	uint8_t width = rect->right - rect->left + 1;
	uint8_t *ch = uigraph::frame_chars;

	vga::set_cursor_pos(rect->top, rect->left);
	vga::write_char(ch[FRAME_TOP], attrs, width);
	vga::write_char(ch[FRAME_TOP_LEFT], attrs, 1);

	vga::set_cursor_pos(rect->top, rect->right);
	vga::write_char(ch[FRAME_TOP_RIGHT], attrs, 1);

	for (int i = rect->top + 1; i < rect->bottom; ++i) {
		vga::set_cursor_pos(i, rect->left);
		vga::write_char(ch[FRAME_LEFT], attrs, 1);

		vga::set_cursor_pos(i, rect->right);
		vga::write_char(ch[FRAME_RIGHT], attrs, 1);
	}

	vga::set_cursor_pos(rect->bottom, rect->left);
	vga::write_char(ch[FRAME_BOTTOM], attrs, width);
	vga::write_char(ch[FRAME_BOTTOM_LEFT], attrs, 1);

	vga::set_cursor_pos(rect->bottom, rect->right);
	vga::write_char(ch[FRAME_BOTTOM_RIGHT], attrs, 1);
}

void uigraph::draw_filled_frame(vga::text_rect_t *rect, uint8_t attrs) {
	graphics::draw_filled_rect(rect, attrs);
	uigraph::draw_frame(rect, attrs);
}

void uigraph::draw_accelerator(uint8_t row, uint8_t col, uint8_t attrs) {
	uigraph::highlight_chars(row, col, attrs & 0x0f, 1);
}

void uigraph::highlight_chars(uint8_t row, uint8_t col, uint8_t attrs,
							  uint16_t count) {
	if (tui::text_mode) {
		if (attrs & 0x80)
			vga::and_attrs(row, col, 0xf0, count);
		else
			vga::or_attrs(row, col, 0x0f, count);
	} else {
		vga::set_cursor_pos(row, col);
		vga::write_char('_', 0x80 | (attrs & 0x0f), count);
	}
}

void uigraph::reverse_item(uint8_t row, uint8_t col, uint8_t attrs,
						   uint16_t count) {
	if (tui::text_mode == TRUE) {
		uint8_t reverse = (attrs & 0x0f) ^
						  ((attrs & 0xf0) >> 4);
		reverse |= reverse << 4;
		vga::xor_attrs(row, col, reverse, count);
	} else {
		vga::set_cursor_pos(row, col);
		vga::write_char(0xdb, 0x80 | (attrs & 0x0f), count);
	}
}

void uigraph::apply_focus(uint8_t row, uint8_t col, uint8_t attrs,
						  uint8_t count) {
	vga::text_rect_t rect;
	rect.top = rect.bottom = row;
	rect.left = col;
	rect.right = col + count - 1;
	uigraph::apply_focus(&rect, attrs, FOCUS_SURFACE);
}

void uigraph::apply_focus(vga::text_rect_t *rect, uint8_t attr,
						  focus_style style) {
	if (tui::text_mode == FALSE) {
		graphics::draw_focus_outline(rect, attr);
		return;
	}

	if (style & FOCUS_SURFACE) {
		uint16_t count = rect->right - rect->left + 1;
		for (uint8_t row = rect->top; row <= rect->bottom; ++row)
			vga::write_attrs(row, rect->left, attr ^ 0x88, count);
	}
}

void uigraph::apply_active(uint8_t row, uint8_t col, uint8_t attrs,
						   uint8_t count) {
	vga::text_rect_t rect;
	rect.top = rect.bottom = row;
	rect.left = col;
	rect.right = col + count - 1;
	uigraph::apply_active(&rect, attrs);
}

void uigraph::apply_active(vga::text_rect_t *rect, uint8_t attrs) {
	uint16_t count = rect->right - rect->left + 1;
	if (tui::text_mode == TRUE) {
		/*if (attrs & 0x80) {
			attrs = 0x80 | ((attrs & 0x0f) ^ 0x0f);
			for (uint8_t row = rect->top; row <= rect->bottom; ++row)
				vga::xor_attrs(row, rect->left, attrs, count);
		} else*/
			for (uint8_t row = rect->top; row <= rect->bottom; ++row)
				vga::or_attrs(row, rect->left, 0x8f, count);
	} else {
		for (uint8_t row = rect->top; row <= rect->bottom; ++row) {
			vga::set_cursor_pos(row, rect->left);
			vga::write_char(0xdb, 0x80 | (attrs & 0x07), count);
		}
	}
}

void uigraph::remove_focus(vga::text_rect_t *rect, uint8_t attrs,
						   focus_style style) {
	if (tui::text_mode == FALSE) {
		graphics::draw_focus_outline(rect, 0);
		graphics::draw_outline(rect, attrs);
		return;
	}
}

void uigraph::draw_shadow(vga::text_rect_t *rect) {
	const vga::state_t *state = vga::get_current_state();
	if (tui::text_mode == TRUE) {
		uint8_t attrs = vga::encode_attrs(uitheme::shadow::back,
										  uitheme::shadow::fore, 0);

		uint8_t left = rect->left, right = rect->right;
		uint8_t bottom = rect->bottom;
		bool_t left_column = FALSE, right_column = FALSE;
		bool_t bottom_row = FALSE;

		if (left > 1) {
			--left;
			left_column = TRUE;
		}

		if (right < state->columns - 1) {
			++right;
			right_column = TRUE;
		}

		if (bottom < state->rows - 1) {
			++bottom;
			bottom_row = TRUE;
		} else
			bottom = state->rows - 1;

		if (left_column) {
			for (uint8_t i = rect->top + 1; i <= bottom; ++i)
				vga::write_attrs(i, left, attrs, 1);
		}

		if (right_column) {
			for (uint8_t i = rect->top + 1; i <= bottom; ++i)
				vga::write_attrs(i, right, attrs, 1);
		}

		if (bottom_row)
			vga::write_attrs(bottom, left, attrs, right - left + 1);
	} else {
		// TODO: render shadow on graphics mode.
	}
}
