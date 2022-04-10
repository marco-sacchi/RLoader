#ifndef TUI_BUTTON_HPP
#define TUI_BUTTON_HPP

#include "types.hpp"

#include "tui/control.hpp"

/** Button control. */
class uibutton : public uicontrol {
public:
	/** Button constructor.
	 *
	 * @param id Control unique identifier.
	 * @param top Zero-based vertical position in characters.
	 * @param left Zero-based horizontal position in characters.
	 * @param width Button width in characters.
	 * @param caption Button caption.
	 */
	uibutton(uint16_t id, int top, int left, int width, const char *caption);
	~uibutton();

	virtual void repaint();

	virtual void on_accelerator(tui::action_t *action);
	virtual void on_message(uiqueue::message_t *msg, tui::action_t *action);

	virtual void lose_focus();

private:
	/** Button caption. */
	char *m_caption;
	/** Button pressed flag. */
	bool_t m_pressed;
};

#endif
