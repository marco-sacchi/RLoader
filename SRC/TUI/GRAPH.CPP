#include "tui/tui.hpp"
#include "tui/graph.hpp"

#include "math.hpp"

void uigraph::enable_caret(bool_t enable) {
	if (tui::text_mode == TRUE)
		vga::enable_cursor(enable);
}

void uigraph::draw_accelerator(uint8_t row, uint8_t col, uint8_t attrs) {
	uigraph::highlight_chars(row, col, attrs, 1);
}

void uigraph::highlight_chars(uint8_t row, uint8_t col, uint8_t attrs,
							  uint16_t count) {
	if (tui::text_mode) {
		vga::xor_attrs(row, col, 0x08, count);
	} else {
		vga::set_cursor_pos(row, col);
		vga::write_char('_', 0x80 | (attrs & 0x0f), count);
	}
}

void uigraph::draw_active_item(uint8_t row, uint8_t col, uint8_t attrs,
							   uint8_t count) {
	if (tui::text_mode == TRUE)
		vga::or_attrs(row, col, 0x88, count);
	else {
		vga::set_cursor_pos(row, col);
		vga::write_char(0xdb, 0x80 | (attrs & 0x07), count);
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

void uigraph::apply_focus(vga::text_rect_t *rect, uint8_t attr,
						  focus_style style) {
	if (tui::text_mode == FALSE) {
		graphics::draw_focus_outline(rect, attr);
		return;
	}

	if (style & FOCUS_SURFACE) {
		uint16_t count = rect->right - rect->left + 1;
		for (uint8_t row = rect->top; row <= rect->bottom; ++row)
			vga::or_attrs(row, rect->left, 0x08, count);
	}
}

void uigraph::remove_focus(vga::text_rect_t *rect, uint8_t attr,
						   focus_style style) {
	if (tui::text_mode == FALSE) {
		graphics::draw_focus_outline(rect, 0);
		graphics::draw_outline(rect, attr);
		return;
	}

	if (style & FOCUS_SURFACE) {
		uint16_t count = rect->right - rect->left + 1;
		for (uint8_t row = rect->top; row <= rect->bottom; ++row)
			vga::and_attrs(row, rect->left, 0xf7, count);
	}
}