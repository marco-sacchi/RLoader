#ifndef TUI_VIEW_HPP
#define TUI_VIEW_HPP

#include "tui/tui.hpp"

// Forward declaration.
class uiwindow;

/** View to present controls and receive action form TUI main loop. */
class uiview {
public:
	/** View state. */
	enum state {
		/** View is allocated but not visible. */
		STATE_INITIALIZING,
		/** View is visible on the screen. */
		STATE_OPENED,
		/** View is closed but not deallocated. */
		STATE_CLOSED,
		/** View, linked window and all descendant control are deallocated. */
		STATE_DESTROYED
	};

	uiview();
	virtual ~uiview();

	/** Get view state.
	 *
	 * @return View state.
	 */
	state get_state() { return m_state; }

	/** Modal view flag status.
	 *
	 * @return True if view is modal, false otherwise.
	 */
	bool_t is_modal() { return m_is_modal; }

	/** Get window control linked to this view.
	 *
	 * @return Window object.
	 */
	uiwindow *get_window() { return m_window; }

	/** Get previous view.
	 *
	 * @return View that opened this one.
	 */
	uiview *get_previous() { return m_previous; }

	/** Called to allocate and initialize al controls.
	 *
	 * @return TRUE on success, FALSE otherwise.
	 */
	virtual bool_t init_controls() = 0;

	/** Called when the view receive an action.
	 *
	 * @param action Received action.
	 * @return TRUE if action is processed, FALSE otherwise.
	 */
	virtual bool_t on_action(tui::action_t *action) = 0;

	/** Open the view displaying it on screen.
	 *
	 * @param previous Previous focused view.
	 * @return TRUE on success, FALSE otherwise.
	 */
	bool_t open(uiview *previous);
	/** Close the view repainting the previous one.
	 *
	 * @param previous Filled with previous view.
	 * @return TRUE on success, FALSE otherwise.
	 */
	bool_t close(uiview **previous);
	/** Called when the view got focus.
	 *
	 * @return TRUE on success, FALSE if state is not STATE_OPENED.
	 */
	bool_t got_focus();

	/** Queue message to be dispatched from TUI main loop.
	 *
	 * @param message Message to be copied into the queue.
	 * @return TRUE on success, FALSE if queue is full.
	 */
	bool_t queue_message(tui::message_t *message);
	/** Dequeue message from view internal queue.
	 *
	 * @param dest Message on top of the queue will be copied here.
	 * @return TRUE on success, FALSE if queue is empty.
	 */
	bool_t dequeue_message(tui::message_t *dest);

protected:
	/** Queue message to open specified view.
	 *
	 * @param view View to open.
	 * @return TRUE on queue success, FALSE if queue is full.
	 */
	bool_t queue_open_view(uiview *view);
	/** Queue message to close this view.
	 *
	 * @return TRUE on queue success, FALSE if queue is full.
	 */
	bool_t queue_close_view();
	/** Queue message to close specified view.
	 *
	 * @param view View to close.
	 * @return TRUE on queue success, FALSE if queue is full.
	 */
	bool_t queue_close_view(uiview *view);

	/** View state. */
	state m_state;
	/** Modal view flag. */
	bool_t m_is_modal;
	/** Window linked to this view. */
	uiwindow *m_window;
	/** Previous view. */
	uiview *m_previous;

private:
	/** Message queue. */
	tui::message_t *m_message_queue;
	/** Number of messages in the queue. */
	int m_message_queue_count;
};

#endif
