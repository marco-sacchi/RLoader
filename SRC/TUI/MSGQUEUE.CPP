#include <mem.h>

#include "tui/msgqueue.hpp"

uiqueue::uiqueue() {
	m_first = m_last = NULL;
}

uiqueue::~uiqueue() {
	if (m_first == NULL)
		return;

	item_t *item = m_first, *next;
	while (item->next != NULL) {
		next = item->next;
		delete item;
		item = next;
	}

	m_first = m_last = NULL;
}

bool_t uiqueue::get_first(message_t *message) {
	if (m_first == NULL)
		return FALSE;

	memcpy(message, &(m_first->data), sizeof(message_t));
	return TRUE;
}

bool_t uiqueue::queue(message_t *message) {
	item_t *item = new item_t();
	if (item == NULL)
		return FALSE;

	memcpy(&(item->data), message, sizeof(message_t));
	item->previous = m_last;
	item->next = NULL;

	if (m_first == NULL) {
		m_first = m_last = item;
		return TRUE;
	}

	m_last->next = item;
	m_last = item;

	return TRUE;
}

bool_t uiqueue::dequeue(message_t *dest) {
	if (m_first == NULL)
		return FALSE;

	memcpy(dest, &(m_first->data), sizeof(message_t));

	item_t *next = m_first->next;
	delete m_first;

	if (next != NULL)
		next->previous = NULL;

	m_first = next;

	if (m_first == NULL)
		m_last = NULL;

	return TRUE;
}
