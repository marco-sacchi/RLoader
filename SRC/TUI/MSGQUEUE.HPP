#ifndef MSGQUEUE_HPP
#define MSGQUEUE_HPP

#include "types.hpp"

class uiqueue {
public:
	/** Message types enumeration. */
	enum message_types {
		/** Keyboard key down. */
		TUIM_KEYDOWN = 1,
		/** Keyboard repeat while key down. */
		TUIM_KEYPRESS,
		/** Keyboard key release. */
		TUIM_KEYUP,

		/** Critical error handler has been triggered. */
		TUIM_CRIT_ERR,

		/** Open new view. */
		TUIM_OPEN_VIEW,
		/** Close view. */
		TUIM_CLOSE_VIEW,
		/** Closed view result. */
		TUIM_VIEW_RESULT
	};

	/** Message sent from low-level code to controls. */
	typedef struct message {
		/** Message type. */
		message_types type;
		/** Message parameter. */
		int param;
		/** Message additional data. */
		void *data;
	} message_t;

	uiqueue();
	~uiqueue();

	/** Return first message without removing if from the queue.
	 *
	 * @return TRUE on success, FALSE if queue is empty.
	 */
	bool_t get_first(message_t *message);

	/** Queue message to be dispatched from TUI main loop.
	 *
	 * @param message Message to be copied into the queue.
	 * @return TRUE on success, FALSE if queue is full.
	 */
	bool_t queue(message_t *message);
	/** Dequeue message from view internal queue.
	 *
	 * @param dest Message on top of the queue will be copied here.
	 * @return TRUE on success, FALSE if queue is empty.
	 */
	bool_t dequeue(message_t *dest);

private:
	typedef struct item {
		message_t data;
		item *previous, *next;
	} item_t;

	item_t *m_first;
	item_t *m_last;
};

#endif MSGQUEUE_HPP
