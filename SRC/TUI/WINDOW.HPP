#ifndef TUI_WINDOW_HPP
#define TUI_WINDOW_HPP

#include "types.hpp"

#include "tui/control.hpp"
/** Window control */
class uiwindow : public uicontrol {
public:
	enum style {
		CAPTIONED = 0x01,
		FRAMED = 0x02,

		CENTERED = 0x10
	};

	/** Window constructor.
	 *
	 * @see uiwindow::style
	 * @param id Control unique identifier.
	 * @param top Zero-based vertical position in characters.
	 * @param left Zero-based horizontal position in characters.
	 * @param width Window width in characters.
	 * @param height Window height in characters.
	 * @param win_style Window style.
	 * @param caption Window caption.
	 */
	uiwindow(uint16_t id, int top, int left, int width, int height,
			 style win_style, const char *caption);
	~uiwindow();

	/** Close the window and dispose all contained controls. */
	void close();

	void got_focus();
	void lose_focus();

	/** Focus next or first focusable contained control. */
	void focus_next_control();

	/** Focus previous focusable contained control. */
	void focus_previous_control();

	/** Focus specific control.
	 *
	 * @param ctrl Control to be focused.
	 */
	bool_t focus_control(uicontrol *ctrl);

	/** Get window caption text/foreground color.
	 *
	 * @return Color index.
	 */
	uint8_t get_caption_fore_color();
	/** Set window caption text/foreground color and repaint control.
	 *
	 * @param color Color index.
	 */
	void set_caption_fore_color(uint8_t color);
	/** Get window caption background color.
	 *
	 * @return Color index.
	 */
	uint8_t get_caption_back_color();
	/** Set window caption background color and repaint control.
	 *
	 * @param color Color index.
	 */
	void set_caption_back_color(uint8_t color);

	virtual void repaint();

	/** Get current focused control.
	 *
	 * @return Pointer to current focused control.
	 */
	uicontrol *get_focused_control() { return m_focused_control; }

	/** Get control by accelerator character.
	 *
	 * @param accelerator Accelerator character.
	 * @param First matching control.
	 */
	uicontrol *get_control_by_accelerator(char accelerator);

protected:
	/** Set window caption text/foreground color.
	 *
	 * @param color Color index.
	 */
	void set_caption_fore_color_internal(uint8_t color);
	/** Set window caption background color.
	 *
	 * @param color Color index.
	 */
	void set_caption_back_color_internal(uint8_t color);

private:
	/** Window style. */
	style m_style;
	/** Window caption. */
	char *m_caption;
	/** Window caption text attributes. */
	uint8_t m_caption_attrs;
	/** Currently focused control. */
	uicontrol *m_focused_control;
};

#endif
