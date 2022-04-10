#include <mem.h>

#include "vga.hpp"
#include "tui/tui.hpp"
#include "tui/theme.hpp"
#include "tui/view.hpp"
#include "tui/window.hpp"

bool_t tui::text_mode;
uiqueue *tui::messages;

tui::tui(bool_t text_mode) {
	m_curr_view = NULL;
	m_view_closing = FALSE;

	tui::text_mode = text_mode;
	tui::messages = new uiqueue();

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

	delete tui::messages;
	tui::messages = NULL;

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

	uiqueue::message_t msg;
	memset(&msg, 0x00, sizeof(uiqueue::message_t));

	switch (this->keyb_data.type) {
		case keyboard::KEV_MAKE:
			msg.type = uiqueue::TUIM_KEYDOWN;
			msg.data = &(this->keyb_data);
			break;

		case keyboard::KEV_PRESS:
			msg.type = uiqueue::TUIM_KEYPRESS;
			msg.data = &(this->keyb_data);
			break;

		case keyboard::KEV_BREAK:
			msg.type = uiqueue::TUIM_KEYUP;
			msg.data = &(this->keyb_data);
			break;
	}

	focused->on_message(&msg, &action);
	m_curr_view->on_action(&action);

	// TODO: need better implementation.
	m_view_closing = FALSE;
	while (m_view_closing == FALSE && tui::messages->dequeue(&msg) == TRUE) {
		switch (msg.type) {
			case uiqueue::TUIM_OPEN_VIEW:
				this->open_view((uiview *)msg.data);
				break;

			case uiqueue::TUIM_CLOSE_VIEW:
				this->close_view((uiview *)msg.data);
				m_view_closing = TRUE;
				break;

			case uiqueue::TUIM_CRIT_ERR:
				int pippo = 3;
				break;
		}
	}

	return TRUE;
}

int tui::modal_loop() {
	while (this->task())
		if (m_view_closing == TRUE)
			break;

	int result = 0;
	uiqueue::message msg;
	tui::messages->get_first(&msg);
	if (msg.type == uiqueue::TUIM_VIEW_RESULT) {
		result = msg.param;
		tui::messages->dequeue(&msg);
	}

	m_view_closing = FALSE;
	return result;
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
