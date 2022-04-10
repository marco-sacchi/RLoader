#include <string.h>

#include "tui/window.hpp"
#include "tui/graph.hpp"

#include "graphics.hpp"
#include "math.hpp"

uiwindow::uiwindow(uint16_t id, int top, int left, int width, int height,
				   uiwindow::style win_style, const char *caption)
 : uicontrol(id) {
	m_style = win_style;

	if (m_style & uiwindow::CAPTIONED) {
		int caption_len = strlen(caption);
		m_caption = new char[caption_len + 3];
		*m_caption = ' ';
		strcpy(m_caption + 1, caption);
		*(m_caption + caption_len + 1) = ' ';
		*(m_caption + caption_len + 2) = NULL;
	} else
		m_caption = NULL;

	if (m_style & uiwindow::CENTERED) {
		const vga::state_t *state = vga::get_current_state();
		top = (state->rows - height) >> 1;
		left = (80 - width) >> 1;
	}

	m_rect.top = top;
	m_rect.left = left;
	m_rect.right = left + width - 1;
	m_rect.bottom = top + height - 1;
	m_focused_control = NULL;

	this->set_back_color_internal(uitheme::window::back);
	this->set_fore_color_internal(uitheme::window::fore);

	this->set_caption_back_color_internal(uitheme::window::caption_back);
	this->set_caption_fore_color_internal(uitheme::window::caption_fore);
}

uiwindow::~uiwindow() {
	if (m_caption != NULL) {
		delete m_caption;
		m_caption = NULL;
	}
}

void uiwindow::close() {

}

void uiwindow::got_focus() {
	uicontrol::got_focus();

	if (m_focused_control == NULL)
		this->focus_next_control();
}

void uiwindow::lose_focus() {
	m_focused_control->lose_focus();
	m_focused_control = NULL;

	uicontrol::lose_focus();
}

void uiwindow::focus_next_control() {
	if (m_focused_control != NULL) {
		m_focused_control->lose_focus();

		do {
			m_focused_control = m_focused_control->get_next();
		} while (m_focused_control != NULL &&
				 m_focused_control->is_focusable() == FALSE);
	}

	if (m_focused_control == NULL) {
		m_focused_control = this->get_first_child();

		while (m_focused_control != NULL &&
			   m_focused_control->is_focusable() == FALSE)
			m_focused_control = m_focused_control->get_next();
	}

	if (m_focused_control != NULL)
		m_focused_control->got_focus();
}

void uiwindow::focus_previous_control() {
	if (m_focused_control != NULL)
		m_focused_control->lose_focus();

	do {
		m_focused_control = m_focused_control->get_previous();
	} while (m_focused_control != NULL &&
			 m_focused_control->is_focusable() == FALSE);

	if (m_focused_control == NULL) {
		m_focused_control = this->get_last_child();

		while (m_focused_control != NULL &&
			   m_focused_control->is_focusable() == FALSE)
			m_focused_control = m_focused_control->get_previous();
	}

	if (m_focused_control != NULL)
		m_focused_control->got_focus();
}

bool_t uiwindow::focus_control(uicontrol *ctrl) {
	if (this->is_child_control(ctrl) == FALSE)
		return FALSE;

	if (m_focused_control != NULL)
		m_focused_control->lose_focus();

	m_focused_control = ctrl;
	m_focused_control->got_focus();
	return TRUE;
}

uicontrol *uiwindow::get_control_by_accelerator(char accelerator) {
	uicontrol *ctrl = this->get_first_child();
	char accel;

	do {
		accel = ctrl->get_accelerator();
		if (ctrl->is_focusable() && accel > 0 && accel == accelerator)
			return ctrl;

	} while ((ctrl = ctrl->get_next()) != NULL);

	return NULL;
}

uint8_t uiwindow::get_caption_fore_color() {
	uint8_t back_color, fore_color, blink;
	vga::decode_attrs(m_caption_attrs, &back_color, &fore_color, &blink);
	return fore_color;
}

void uiwindow::set_caption_fore_color(uint8_t color) {
	// Cannot change color on 1-bit graphics UI.
	if (tui::text_mode == FALSE)
		return;

	this->set_caption_fore_color_internal(color);
	this->repaint();
}

void uiwindow::set_caption_fore_color_internal(uint8_t color) {
	uint8_t back_color, fore_color, blink;
	vga::decode_attrs(m_caption_attrs, &back_color, &fore_color, &blink);
	m_caption_attrs = vga::encode_attrs(back_color, color, 0);
}

uint8_t uiwindow::get_caption_back_color() {
	uint8_t back_color, fore_color, blink;
	vga::decode_attrs(m_caption_attrs, &back_color, &fore_color, &blink);
	return back_color;
}

void uiwindow::set_caption_back_color(uint8_t color) {
	// Cannot change color on 1-bit graphics UI.
	if (tui::text_mode == FALSE)
		return;

	this->set_caption_back_color_internal(color);
	this->repaint();
}

void uiwindow::set_caption_back_color_internal(uint8_t color) {
	uint8_t back_color, fore_color, blink;
	vga::decode_attrs(m_caption_attrs, &back_color, &fore_color, &blink);
	m_caption_attrs = vga::encode_attrs(color, fore_color, 0);
}

void uiwindow::repaint() {
	if (this->can_repaint() == FALSE) {
		m_repaint_pending = TRUE;
		return;
	}

	vga::text_rect_t rect;
	this->get_paint_rect(&rect);

	graphics::draw_filled_rect(&rect, m_paint_attrs);

	if (m_style & uiwindow::FRAMED)
		graphics::draw_frame(&rect, m_paint_attrs);
	else if (tui::text_mode == FALSE)
		graphics::draw_outline(&rect, m_paint_attrs);

	if (m_style & uiwindow::CAPTIONED) {
		uint8_t rect_width = rect.right - rect.left + 1;
		uint8_t caption_length = min(rect_width, (uint8_t)strlen(m_caption));
		uint8_t caption_start = (rect_width >> 1) - (caption_length >> 1);

		vga::set_cursor_pos(rect.top, rect.left + caption_start);
		vga::write_string(m_caption, caption_length, m_paint_attrs,
						  vga::USE_ATTRS_UPDATE_CURSOR);

		if (!(m_style & uiwindow::FRAMED)) {
			if (tui::text_mode == TRUE)
				vga::write_attrs(rect.top, rect.left, m_caption_attrs,
								 rect_width);
			else
				uigraph::reverse_item(rect.top, rect.left, m_paint_attrs, rect_width);
		}
	}

	// Repaint descendants.
	uicontrol::repaint();
}
