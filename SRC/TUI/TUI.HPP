#ifndef TUI_HPP
#define TUI_HPP

#include "types.hpp"

#include "keyboard.hpp"

#include "tui/msgqueue.hpp"

// Forward declarations.
class uicontrol;
class uiview;

/** Text-based and graphics user interface. */
class tui {
public:
	/** Control actions enumeration. */
	enum action_types {
		/** A view is closed. */
		TUIA_VIEW_CLOSED = 1,
		/** Control activated by ENTER key. */
		TUIA_CTRL_ACTIVATED,
		/** New control is focused on current view. */
		TUIA_CTRL_FOCUS,
		/** Control is destroyed. */
		TUIA_CTRL_DESTROY,
		/** Control value is changed. */
		TUIA_CTRL_VALUE_CHANGED,
		/** Control item selection is changing. */
		TUIA_ITEM_SELECTING,
		/** Control item is selected, triggered on keyboard key release. */
		TUIA_ITEM_SELECTED,
		/** Control item activated by ENTER key. */
		TUIA_ITEM_ACTIVATED
	};

	/** Action returned by controls when a message is processed. */
	typedef struct {
		/** Action type. */
		action_types type;
		/** Control id. */
		uint16_t id;
		/** Item id, if the action is on a control item. */
		uint16_t item_id;
		/** Pointer to control that has generated the action. */
		uicontrol *control;
		/** Pointer to view that has generated the action, for view actions. */
		uiview *view;
	} action_t;

	/** User-interface initialization.
	 *
	 * @param text_mode TRUE for text-mode UI, FALSE for graphics UI.
	 */
	tui(bool_t text_mode);
	~tui();

	/** Run message and actions handling task.
	 *
	 * @return TRUE if event is processed, FALSE if loop must terminate.
	 */
	bool_t task();

	/** Loop modally until last opened view is closed then return.
	 *
	 * @return View result value if view queue TUIM_VIEW_RESULT event,
	 * zero otherwise.
	 */
	int modal_loop();

	/** Open specified view.
	 *
	 * @param view View to be opened and made current.
	 * @return TRUE on correct initialization and open, FALSE otherwise.
	 */
	bool_t open_view(uiview *view);
	/** Close specified view.
	 *
	 * @param view View to be closed, previous one becomes current.
	 * @return TRUE on correct initialization and open, FALSE otherwise.
	 */
	bool_t close_view(uiview *view);

	/** Used on TUI uigraph.
	 *
	 * @note It must only be read.
	 */
	static bool_t text_mode;

	static uiqueue *messages;
private:
	/** Currently active view. */
	uiview *m_curr_view;
	/** Data read from keyboard. */
	keyboard::read_data_t keyb_data;
	/** Track if topmost view is closed for modal_loop method. */
	bool_t m_view_closing;
};

#endif
