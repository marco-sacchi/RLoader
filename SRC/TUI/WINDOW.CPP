#include <string.h>

#include "tui/window.hpp"

#include "graphics.hpp"
#include "math.hpp"

uiwindow::uiwindow(uint16_t id, int top, int left, int width, int height,
				   const char *caption)
 : uicontrol(id) {
	m_caption = new char[strlen(caption) + 1];
	strcpy(m_caption, caption);
	m_rect.top = top;
	m_rect.left = left;
	m_rect.right = left + width - 1;
	m_rect.bottom = top + height - 1;
	m_focused_control = NULL;
}

uiwindow::~uiwindow() {
	delete m_caption;
	m_caption = NULL;
}

void uiwindow::close() {

}

void uiwindow::got_focus() {
	uicontrol::got_focus();

	if (m_focused_control == NULL)
		m_focused_control = this->get_first_child();

	m_focused_control->got_focus();
}

void uiwindow::lose_focus() {
	m_focused_control->lose_focus();
	m_focused_control = NULL;

	uicontrol::lose_focus();
}

void uiwindow::focus_next_control() {
	if (m_focused_control != NULL)
		m_focused_control->lose_focus();

	do {
		m_focused_control = m_focused_control->get_next();
	} while (m_focused_control != NULL &&
			 m_focused_control->is_focusable() == FALSE);

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

void uiwindow::repaint() {
	vga::text_rect_t rect;
	this->get_paint_rect(&rect);

	graphics::draw_filled_frame(&rect, 0x07);
	uint8_t rect_width = rect.right - rect.left + 1;
	uint8_t caption_length = min(rect_width, (uint8_t)strlen(m_caption));
	uint8_t caption_start = (rect_width >> 1) - (caption_length >> 1);

	vga::set_cursor_pos(rect.top, rect.left + caption_start - 1);
	vga::write_char(' ', 0x07, 1);

	vga::set_cursor_pos(rect.top, rect.left + caption_start);
	vga::write_string(m_caption, caption_length, 0x07,
					  vga::USE_ATTRS_UPDATE_CURSOR);

	vga::write_char(' ', 0x07, 1);

	// Repaint descendants.
	uicontrol::repaint();
}
