#ifndef TUI_EDITBOX_HPP
#define TUI_EDITBOX_HPP

#include "types.hpp"

#include "tui/control.hpp"

/** Editbox control. */
class uieditbox : public uicontrol {
public:
	/** Editbox constructor.
	 *
	 * @param id Control unique identifier.
	 * @param top Zero-based vertical position in characters.
	 * @param left Zero-based horizontal position in characters.
	 * @param width List width in characters, scrollbar included.
	 * @param max_length Maximum allwed text length.
	 */
	uieditbox(uint16_t id, int top, int left, uint16_t width,
			  uint16_t max_length);
	~uieditbox();

	/** Get displayed text.
	 *
	 * @return Displayed text.
	 */
	const char *get_text();

	/** Set displayed text.
	 *
	 * @param text Text to display.
	 */
	void set_text(const char *text);

	virtual void repaint();

	virtual void got_focus();
	virtual void lose_focus();

	virtual void on_message(uiqueue::message_t *msg, tui::action_t *action);

private:
	/** Render caret at specified position.
	 *
	 * @param pos Caret position as zero-based character index.
	 */
	void render_caret(uint16_t pos);

	/** Displayed text. */
	char *m_text;
	/** Maximum allowed text length. */
	uint16_t m_max_length;
	/** Current text display offset in characters. */
	uint16_t m_text_offset;
	/** Current caret position. */
	uint16_t m_caret_pos;
	/** True when a selection range is active. */
	bool_t m_selected;
	/** Position of selection range oppsed to the caret position. */
	uint16_t m_selection_pos;
};

#endif
