#include <dos.h>
#include <string.h>

#include "tui/button.hpp"
#include "tui/graph.hpp"
#include "math.hpp"
#include "string.hpp"

uibutton::uibutton(uint16_t id, int top, int left, int width,
				   const char *caption)
 : uicontrol(id) {
	int caption_length = strlen(caption);
	m_caption = new char[caption_length + 1];
	strcpy(m_caption, caption);

	m_pressed = FALSE;

	width = max(width, caption_length);
	m_rect.top = m_rect.bottom = top;
	m_rect.left = left;
	m_rect.right = left + width - 1;

	this->set_back_color_internal(uitheme::button::back);
	this->set_fore_color_internal(uitheme::button::fore);
}

uibutton::~uibutton() {
	delete m_caption;
	m_caption = NULL;
}

void uibutton::lose_focus() {
	// If button is pressed when lose focus, revert it to normal state
	// to avoid unwanted activation.
	if (m_pressed == TRUE) {
		m_pressed = FALSE;
		this->repaint();
	}

	uicontrol::lose_focus();
}

void uibutton::repaint() {
	if (this->can_repaint() == FALSE) {
		m_repaint_pending = TRUE;
		return;
	}

	vga::text_rect_t rect;
	this->get_paint_rect(&rect);

	vga::set_cursor_pos(rect.top, rect.left);
	vga::write_char(' ', m_paint_attrs, rect.right - rect.left + 1);

	int offset = ((rect.right - rect.left + 1) >> 1) -
				 (strlen(m_caption) >> 1);
	vga::set_cursor_pos(rect.top, rect.left + offset);
	vga::write_string(m_caption, m_paint_attrs,
					  vga::USE_ATTRS_NO_UPDATE_CURSOR);

	if (m_focused == TRUE)
		uigraph::apply_focus(&rect, m_paint_attrs, uigraph::FOCUS_SURFACE);
	else
		uigraph::remove_focus(&rect, m_paint_attrs, uigraph::FOCUS_SURFACE);

	if (m_accelerator != NULL && m_caption != NULL) {
		char *ch = m_caption;
		int accel_index = 0;
		while (*ch != NULL) {
			if (string::icase_compare(*ch, m_accelerator) == TRUE) {
				offset += accel_index + rect.left;
				uigraph::draw_accelerator(rect.top, offset, m_paint_attrs);
				break;
			}

			++accel_index;
			++ch;
		}
	}

	if (m_pressed == TRUE)
		uigraph::apply_active(&rect, m_paint_attrs);

	uicontrol::repaint();
}

void uibutton::on_accelerator(tui::action_t *action) {
	m_pressed = TRUE;
	this->repaint();

	delay(250);
	m_pressed = FALSE;
	this->repaint();

	action->type = tui::TUIA_CTRL_ACTIVATED;
	action->id = m_id;
	action->control = this;
}

void uibutton::on_message(uiqueue::message_t *msg, tui::action_t *action) {
	if (msg->type == uiqueue::TUIM_KEYDOWN && m_pressed == FALSE) {
		keyboard::read_data_t *data = (keyboard::read_data_t *)msg->data;
		bool_t triggered = (data->virt_key == VK_SPACE ||
							data->virt_key == VK_ENTER);

		if (triggered && data->shifts == 0) {
			m_pressed = TRUE;
			this->repaint();
		}
	} else if (msg->type == uiqueue::TUIM_KEYUP && m_pressed == TRUE) {
		keyboard::read_data_t *data = (keyboard::read_data_t *)msg->data;
		bool_t triggered = (data->virt_key == VK_SPACE ||
							data->virt_key == VK_ENTER ||
							data->virt_key == VK_ESC);

		if (triggered && data->shifts == 0) {
			m_pressed = FALSE;
			this->repaint();

			if (data->virt_key != VK_ESC) {
				action->type = tui::TUIA_CTRL_ACTIVATED;
				action->id = m_id;
				action->control = this;
			}
		}
	}
}
