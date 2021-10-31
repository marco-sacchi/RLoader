#include <string.h>

#include "tui/editbox.hpp"
#include "tui/graph.hpp"
#include "math.hpp"

uieditbox::uieditbox(uint16_t id, int top, int left, uint16_t width,
					 uint16_t max_length)
 : uicontrol(id) {
	m_max_length = max_length;

	m_text = new char[m_max_length + 1];
	memset(m_text, 0x00, m_max_length + 1);

	m_rect.top = m_rect.bottom = top;
	m_rect.left = left;
	m_rect.right = left + width - 1;

	m_text_offset = 0;
	m_caret_pos = 0;
	m_selection_pos = 0;
	m_selected = FALSE;

	// DEBUG: temp debug values.
	this->set_fore_color(7);
	this->set_back_color(1);
}

uieditbox::~uieditbox() {
	delete m_text;
	m_text = NULL;
}

const char *uieditbox::get_text() {
	return m_text;
}

void uieditbox::set_text(const char *text) {
	int text_length = strlen(m_text);

	strncpy(m_text, text, m_max_length);

	int new_text_length = min(m_max_length, (uint16_t)strlen(m_text));
	*(m_text + new_text_length) = NULL;

	int field_width = m_rect.right - m_rect.left - 1;

	// Scroll left automatically when new text is shorter.
	if (new_text_length < text_length && m_text_offset > 0 &&
		((text_length - m_text_offset) > (field_width - 1)))
		m_text_offset = max(0, text_length - field_width + 1);
	else
		m_text_offset = max(0, new_text_length - field_width + 1);

	m_caret_pos = new_text_length;

	this->repaint();
}

void uieditbox::got_focus() {
	uicontrol::got_focus();
	uigraph::enable_caret(TRUE);
}

void uieditbox::lose_focus() {
	uicontrol::lose_focus();
	uigraph::enable_caret(FALSE);
}

void uieditbox::repaint() {
	if (this->can_repaint() == FALSE)
		return;

	vga::text_rect_t rect;
	this->get_paint_rect(&rect);

	int field_width = rect.right - rect.left - 1;
	int text_length = strlen(m_text);

	bool_t left_overflow = (m_text_offset > 0);
	bool_t right_overflow = (field_width < text_length - m_text_offset);

	// Render left overflow indicator.
	vga::set_cursor_pos(rect.top, rect.left);
	vga::write_char((left_overflow == TRUE) ? '®' : ' ', m_paint_attrs, 1);

	// Render right overflow indicator.
	vga::set_cursor_pos(rect.top, rect.right);
	vga::write_char((right_overflow == TRUE) ? '¯' : ' ', m_paint_attrs, 1);

	text_length -= m_text_offset;
	if (text_length > field_width)
		text_length = field_width;

	if (text_length >= 0) {
		vga::set_cursor_pos(rect.top, rect.left + 1);
		vga::write_string(m_text + m_text_offset, text_length, m_paint_attrs,
						  vga::USE_ATTRS_UPDATE_CURSOR);

		if (text_length < field_width)
			vga::write_char(' ', m_paint_attrs, field_width - text_length);
	}

	this->render_caret(m_caret_pos);

	if (m_focused == TRUE)
		uigraph::apply_focus(&rect, m_paint_attrs, uigraph::FOCUS_SURFACE);
	else
		uigraph::remove_focus(&rect, m_paint_attrs, uigraph::FOCUS_SURFACE);

	uicontrol::repaint();
}

void uieditbox::render_caret(uint16_t pos) {
	if (m_focused == FALSE || this->can_repaint() == FALSE)
		return;

	vga::text_rect_t rect;
	this->get_paint_rect(&rect);

	vga::set_cursor_pos(rect.top, rect.left + 1 + pos - m_text_offset);
	if (tui::text_mode == FALSE)
		vga::write_char('_', 0x80 | (m_paint_attrs & 0x07), 1);
}

void uieditbox::on_message(tui::message_t *msg, tui::action_t *action) {
	if (msg->type == tui::TUIM_KEYPRESS) {
		keyboard::read_data_t *data = (keyboard::read_data_t *)msg->data;

		uint16_t prev_caret_pos = m_caret_pos;
		uint16_t prev_text_offset = m_text_offset;
		bool_t text_changed = FALSE;
		int text_length = strlen(m_text);

		switch (data->virt_key) {
			case VK_HOME:
				m_caret_pos = 0;
				break;

			case VK_END:
				m_caret_pos = text_length;
				break;

			case VK_LEFT:
				// TODO: check for CTRL pressed to move by words
				// instead of chars.
				if (m_caret_pos > 0)
					--m_caret_pos;
				break;

			case VK_RIGHT:
				// TODO: check for CTRL pressed to move by words
				// instead of chars.
				if (m_caret_pos < text_length)
					++m_caret_pos;
				break;

			case VK_BKSPC:
				if (text_length > 0) {
					if (m_caret_pos < text_length) {
						int shift_length = 1 + text_length - m_caret_pos;
						memmove(m_text + m_caret_pos - 1,
								m_text + m_caret_pos,
								shift_length);
					} else
						*(m_text + m_caret_pos - 1) = 0;

					--m_caret_pos;

					text_changed = TRUE;
				}
				break;

			case VK_DEL:
				if (m_caret_pos < text_length) {
					int shift_length = 1 + text_length - m_caret_pos;
					memmove(m_text + m_caret_pos,
							m_text + m_caret_pos + 1,
							shift_length);

					text_changed = TRUE;
				}
				break;

			default:
				if (data->ascii >= 0x20) {
					if (text_length < m_max_length) {
						if ((data->locked_shifts & VS_INSERT) &&
							m_caret_pos < text_length) {
							int shift_length = 1 + text_length - m_caret_pos;
							memmove(m_text + m_caret_pos + 1,
									m_text + m_caret_pos,
									shift_length);
						}

						*(m_text + m_caret_pos) = data->ascii;
						++m_caret_pos;

						text_changed = TRUE;
					}
				}
				break;
		}

		uint16_t field_width = m_rect.right - m_rect.left - 1;

		if (prev_caret_pos != m_caret_pos || text_changed == TRUE) {
			if (m_caret_pos < m_text_offset)
				m_text_offset = m_caret_pos;

			if (m_caret_pos > (m_text_offset + field_width - 1))
				m_text_offset = m_caret_pos - (field_width - 1);

			if (prev_text_offset == m_text_offset && text_changed == FALSE) {
				this->render_caret(prev_caret_pos);
				this->render_caret(m_caret_pos);
			} else
				this->repaint();
		}

		if (text_changed == TRUE) {
			action->type = tui::TUIA_CTRL_VALUE_CHANGED;
			action->id = m_id;
			action->control = this;
		}
	}
}
