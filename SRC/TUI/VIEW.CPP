#include <mem.h>

#include "tui/view.hpp"
#include "tui/window.hpp"

#define MAX_QUEUE_SIZE 8

uiview::uiview() {
	m_is_modal = FALSE;

	m_window = NULL;
	m_state = STATE_INITIALIZING;
	m_previous = NULL;

	m_message_queue = new tui::message_t[MAX_QUEUE_SIZE];
	m_message_queue_count = 0;
}

uiview::~uiview() {
	if (m_window != NULL) {
		delete m_window;
		m_window = NULL;
	}

	if (m_message_queue != NULL) {
		delete m_message_queue;
		m_message_queue = NULL;
	}

	m_state = STATE_DESTROYED;
	m_previous = NULL;
}

bool_t uiview::open(uiview *previous) {
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

bool_t uiview::queue_message(tui::message_t *message) {
	if (m_message_queue_count >= MAX_QUEUE_SIZE)
		return FALSE;

	memcpy(&m_message_queue[m_message_queue_count++],
		   message, sizeof(tui::message_t));

	return TRUE;
}

bool_t uiview::dequeue_message(tui::message_t *dest) {
	if (m_message_queue_count == 0)
		return FALSE;

	memcpy(&m_message_queue[m_message_queue_count],
		   m_message_queue, sizeof(tui::message_t));

	memcpy(dest, m_message_queue, sizeof(tui::message_t));

	if (--m_message_queue_count > 0)
		memmove(m_message_queue, &m_message_queue[1],
				sizeof(tui::message_t) * m_message_queue_count);

	return TRUE;
}
