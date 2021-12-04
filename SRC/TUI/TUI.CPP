#include <mem.h>

#include "vga.hpp"
#include "tui/tui.hpp"
#include "tui/theme.hpp"
#include "tui/view.hpp"
#include "tui/window.hpp"

bool_t tui::text_mode;

tui::tui(bool_t text_mode) {
	m_curr_view = NULL;
	tui::text_mode = text_mode;

	uitheme::init(text_mode);

	vga::set_blinking(FALSE);
	vga::enable_cursor(FALSE);
}

tui::~tui() {
	uiview *view = (uiview *)m_curr_view;
	while (view != NULL) {
		uiview *previous = (uiview *)view->get_previous();
		delete view;
		view = previous;
	}

	m_curr_view = NULL;

	vga::set_blinking(TRUE);
	vga::enable_cursor(TRUE);
}

bool_t tui::task() {
	if (m_curr_view == NULL)
		return FALSE;

	if (!keyboard::read(&(this->keyb_data)))
		return TRUE;

	uiwindow *curr_window = m_curr_view->get_window();
	uicontrol *focused = curr_window->get_focused_control();
	if (focused == NULL)
		return FALSE;

	action_t action;
	memset(&action, 0x00, sizeof(tui::action_t));

	if (m_curr_view->is_modal() == FALSE &&
		this->keyb_data.type == keyboard::KEV_MAKE &&
		this->keyb_data.virt_key == VK_F3 &&
		((this->keyb_data.shifts & VS_ALT) &&
		((this->keyb_data.shifts & ~VS_ALT) == 0))) {

		this->close_view(m_curr_view);

		return TRUE;
	} else if (this->keyb_data.type == keyboard::KEV_PRESS &&
			   this->keyb_data.virt_key == VK_TAB) {
		if (this->keyb_data.shifts & VS_SHIFT &&
			(this->keyb_data.shifts & ~VS_SHIFT) == 0)
			curr_window->focus_previous_control();
		else
			curr_window->focus_next_control();

		focused = curr_window->get_focused_control();

		action.type = TUIA_CTRL_FOCUS;
		action.id = focused->get_id();
		action.control = focused;

		m_curr_view->on_action(&action);

		return TRUE;
	}

	if (this->keyb_data.type == keyboard::KEV_MAKE &&
		this->keyb_data.shifts == VS_LALT) {
		uicontrol *accel_control = NULL;
		char ch = keyboard::virtual_key_to_char(this->keyb_data.virt_key);
		if (ch > 0)
			accel_control = curr_window->get_control_by_accelerator(ch);

		if (accel_control != NULL) {
			curr_window->focus_control(accel_control);
			accel_control->on_accelerator(&action);
			m_curr_view->on_action(&action);
			return TRUE;
		}
	}

	tui::message_t msg;
	memset(&msg, 0x00, sizeof(tui::message_t));

	switch (this->keyb_data.type) {
		case keyboard::KEV_MAKE:
			msg.type = tui::TUIM_KEYDOWN;
			msg.data = &(this->keyb_data);
			break;

		case keyboard::KEV_PRESS:
			msg.type = tui::TUIM_KEYPRESS;
			msg.data = &(this->keyb_data);
			break;

		case keyboard::KEV_BREAK:
			msg.type = tui::TUIM_KEYUP;
			msg.data = &(this->keyb_data);
			break;
	}

	focused->on_message(&msg, &action);
	m_curr_view->on_action(&action);

	// TODO: need better implementation.
	while (m_curr_view != NULL &&
		   m_curr_view->dequeue_message(&msg) == TRUE) {
		if (msg.type == TUIM_OPEN_VIEW) {
			this->open_view((uiview *)msg.data);
		} else if (msg.type == TUIM_CLOSE_VIEW) {
			this->close_view((uiview *)msg.data);
		}
	}

	return TRUE;
}

bool_t tui::open_view(uiview *view) {
	if (view->open(m_curr_view) == FALSE) {
		delete view;
		return FALSE;
	}

	m_curr_view = view;

	return TRUE;
}

bool_t tui::close_view(uiview *view) {
	action_t action;
	action.type = TUIA_CTRL_DESTROY;
	action.id = view->get_window()->get_id();

	if (view->on_action(&action) != TRUE)
		return FALSE;

	uiview *previous = NULL;
	if (view->close(&previous) == TRUE)
		m_curr_view = previous;

	if (previous != NULL) {
		memset(&action, 0x00, sizeof(action_t));
		action.type = TUIA_VIEW_CLOSED;
		action.view = view;
		previous->on_action(&action);
	}

	delete view;
	return TRUE;
}
