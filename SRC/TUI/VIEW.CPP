#include <mem.h>

#include "tui/view.hpp"
#include "tui/window.hpp"

#define MAX_QUEUE_SIZE 8

uiview::uiview() {
	m_is_modal = FALSE;

	m_window = NULL;
	m_state = STATE_INITIALIZING;
	m_previous = NULL;
}

uiview::~uiview() {
	if (m_window != NULL) {
		delete m_window;
		m_window = NULL;
	}

	m_state = STATE_DESTROYED;
	m_previous = NULL;
}

bool_t uiview::open(uiview *previous) {
	if (m_state != STATE_INITIALIZING)
		return FALSE;

	if (this->init_controls() == FALSE)
		return FALSE;

	m_previous = previous;

	m_window->set_view(this);
	m_state = STATE_OPENED;

	this->got_focus();

	return TRUE;
}

bool_t uiview::close(uiview **previous) {
	if (m_state != STATE_OPENED)
		return FALSE;

	m_state = STATE_CLOSED;

	m_window->close();

	if (m_previous != NULL)
		m_previous->got_focus();

	*previous = m_previous;

	return TRUE;
}

bool_t uiview::got_focus() {
	if (m_state != STATE_OPENED)
		return FALSE;

	m_window->repaint();
	m_window->got_focus();

	return TRUE;
}

bool_t uiview::queue_open_view(uiview *view) {
	uiqueue::message_t msg;
	msg.type = uiqueue::TUIM_OPEN_VIEW;
	msg.data = view;
	return tui::messages->queue(&msg);
}

bool_t uiview::queue_close_view() {
	return this->queue_close_view(this);
}

bool_t uiview::queue_close_view(uiview *view) {
	uiqueue::message_t msg;
	msg.type = uiqueue::TUIM_CLOSE_VIEW;
	msg.data = view;
	return tui::messages->queue(&msg);
}

bool_t uiview::queue_result(int result) {
	uiqueue::message_t msg;
	msg.type = uiqueue::TUIM_VIEW_RESULT;
	msg.param = result;
	msg.data = NULL;
	return tui::messages->queue(&msg);
}
