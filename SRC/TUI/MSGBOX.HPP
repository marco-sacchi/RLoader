#ifndef MSGBOX_HPP
#define MSGBOX_HPP

#include "tui/view.hpp"

#define MSGBOX_MAX_BUTTONS	8

// Forward declarations.
class uibutton;

class uimsgbox : public uiview {
public:
	/** Standard buttons id. */
	enum buttons {
		BTN_OK,
		BTN_CANCEL,
		BTN_YES,
		BTN_NO,
		BTN_RETRY,
		BTN_ABORT,
		/** First available id for custom buttons. */
		BTN_CUSTOM_FIRST
	};

	uimsgbox(const char *caption, const char *message);
	virtual ~uimsgbox();

	bool_t add_button(int id);
	bool_t add_button(int id, const char *caption, char accelerator);

	virtual bool_t init_controls();

	virtual bool_t on_action(tui::action_t *action);

	int get_selected_button_id() { return m_selected_button_id; }

private:
	/** Structure to track buttons added to the message box. */
	typedef struct {
		/** Button id, @see uimsgbox::buttons for standard button id. */
		int id;
		/** Button caption, NULL for standard buttons. */
		char *caption;
		/** Button accelerator, 0 for standard buttons. */
		char accelerator;
		/** Left offset from column calculated for buttons positioning. */
		int left;
		/** Button width. */
		int width;
	} button_info_t;

	/** Calculate button properties like position and size.
	 *
	 * @return Total buttons width in characters, from left edge
	 *         of the first one to right edge of the last one.
	 */
	int calc_buttons_props();

	int wrap_message(int max_width);

	/** Captions of standard buttons. */
	static const char *m_captions[];
	/** Accelerators of standard buttons. */
	static const char m_accelerators[];

	/** Message box caption. */
	char *m_caption;
	/** Message box message text. */
	char *m_message;
	/** Number of buttons in message box. */
	int m_buttons_count;
	/** Buttons infos, @see uimsgbox::button_info_t */
	button_info_t m_buttons_infos[MSGBOX_MAX_BUTTONS];
	/** Selected button id, set before closing the messagebox. */
	int m_selected_button_id;
};

#endif
