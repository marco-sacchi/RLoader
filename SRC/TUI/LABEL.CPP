#include <string.h>

#include "tui/label.hpp"
#include "tui/graph.hpp"
#include "math.hpp"
#include "string.hpp"

uilabel::uilabel(uint16_t id, int top, int left, const char *text)
 : uicontrol(id) {
	m_focusable = FALSE;

	m_autosize = TRUE;
	m_multiline = FALSE;

	m_rect.top = m_rect.bottom = top;
	m_rect.left = left;

	m_text = NULL;
	m_lines = NULL;

	this->set_text(text);

	this->set_fore_color_internal(uitheme::label::fore);

	// Label is transparent by default.
	m_transparent = TRUE;
}

uilabel::uilabel(uint16_t id, int top, int left, int width, int height,
				 const char *text)
 : uicontrol(id) {
	m_focusable = FALSE;

	m_multiline = TRUE;

	m_rect.bottom = m_rect.top = top;
	m_rect.left = left;
	m_rect.right = left + width - 1;

	if (height > 0) {
		m_autosize = FALSE;
		m_rect.bottom += height - 1;
	} else {
		m_autosize = TRUE;
	}

	m_text = NULL;
	m_lines = NULL;

	this->set_text(text);

	m_paint_attrs = vga::encode_attrs(0, uitheme::label::fore, 0);

	// Label is transparent by default.
	m_transparent = TRUE;
}

uilabel::~uilabel() {
	if (m_lines != NULL)
		delete m_lines;

	delete m_text;
	m_text = NULL;
}

bool_t uilabel::set_text(const char *text) {
	int text_length = strlen(text);
	char *new_text = new char[text_length + 1];
	if (new_text == NULL)
		return FALSE;

	if (m_multiline == TRUE) {
		int height;
		if (m_autosize == TRUE) {
			const vga::state_t *state = vga::get_current_state();
			height = state->rows;
		} else {
			height = m_rect.bottom - m_rect.top + 1;
		}

		char **new_lines = new char *[height];
		if (new_lines == NULL) {
			delete new_text;
			return FALSE;
		}

		if (m_text != NULL)
			delete m_text;
		if (m_lines != NULL)
			delete m_lines;

		m_text = new_text;
		strcpy(m_text, text);

		m_lines = new_lines;
		m_lines_count = 0;

		m_effective_width = this->wrap_text(height);
		m_rect.bottom = m_rect.top + m_lines_count - 1;
	} else {
		if (m_text != NULL)
			delete m_text;
		if (m_lines != NULL)
			delete m_lines;

		m_text = new_text;
		strcpy(m_text, text);

		m_lines = NULL;
		m_lines_count = 0;

		m_rect.right = m_rect.left + text_length - 1;
	}

	this->repaint();

	return TRUE;
}

int uilabel::wrap_text(int max_lines) {
	int effective_width = 0;
	int length = strlen(m_text);
	int max_width = m_rect.right - m_rect.left + 1;

	m_lines_count = 0;
	char *ptr = m_text;

	while (m_lines_count < max_lines && (ptr + max_width - m_text) < length) {
		char *split_ptr = ptr + max_width;
		while (split_ptr > ptr && !string::is_white_space(*split_ptr))
			--split_ptr;

		while (split_ptr > ptr && string::is_white_space(*split_ptr))
			--split_ptr;

		*(split_ptr + 1) = NULL;
		split_ptr += 2;
		while (string::is_white_space(*split_ptr))
			++split_ptr;

		effective_width = max(effective_width, (int)strlen(ptr));

		m_lines[m_lines_count++] = ptr;
		ptr = split_ptr;
	}

	if (m_lines_count < max_lines)
		m_lines[m_lines_count++] = ptr;

	return effective_width;
}

void uilabel::repaint() {
	if (this->can_repaint() == FALSE) {
		m_repaint_pending = TRUE;
		return;
	}

	this->update_transparency_attrs();

	vga::text_rect_t rect;
	this->get_paint_rect(&rect);

	graphics::draw_filled_rect(&rect, m_paint_attrs);

	int accel_index = -1, accel_line = 0;

	if (m_multiline == TRUE) {
		int row = rect.top;
		for (int i = 0; i < m_lines_count; ++i, ++row) {
			vga::set_cursor_pos(row, rect.left);
			vga::write_string(m_lines[i], m_paint_attrs,
							  vga::USE_ATTRS_NO_UPDATE_CURSOR);

			if (m_accelerator != NULL && accel_index == -1) {
				accel_index = string::icase_index_of(m_lines[i],
													 m_accelerator);
				accel_line = i;
			}
		}
	} else {
		vga::set_cursor_pos(rect.top, rect.left);
		vga::write_string(m_text, m_paint_attrs,
						  vga::USE_ATTRS_NO_UPDATE_CURSOR);

		if (m_accelerator != NULL)
			accel_index = string::icase_index_of(m_text, m_accelerator);
	}

	if (accel_index >= 0) {
		accel_line += rect.top;
		accel_index += rect.left;
		uigraph::draw_accelerator(accel_line, accel_index, m_paint_attrs);
	}

	uicontrol::repaint();
}
