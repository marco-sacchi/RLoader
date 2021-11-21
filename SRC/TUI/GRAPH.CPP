#include "tui/tui.hpp"
#include "tui/graph.hpp"

#include "math.hpp"

void uigraph::enable_caret(bool_t enable) {
	if (tui::text_mode == TRUE)
		vga::enable_cursor(enable);
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

	if (style & FOCUS_SURFACE) {
		uint16_t count = rect->right - rect->left + 1;
		for (uint8_t row = rect->top; row <= rect->bottom; ++row)
			vga::and_attrs(row, rect->left, 0xf7, count);
	}
}