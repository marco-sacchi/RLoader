#ifndef TUI_LABEL_HPP
#define TUI_LABEL_HPP

#include "types.hpp"

#include "tui/control.hpp"

/** Label control. */
class uilabel : public uicontrol {
public:
	/** Label constructor.
	 *
	 * @param id Control unique identifier.
	 * @param top Zero-based vertical position in characters.
	 * @param left Zero-based horizontal position in characters.
	 * @param text Label displayed text.
	 */
	uilabel(uint16_t id, int top, int left, const char *text);
	/** Label constructor.
	 *
	 * @param id Control unique identifier.
	 * @param top Zero-based vertical position in characters.
	 * @param left Zero-based horizontal position in characters.
	 * @param width Width in characters.
	 * @param height Height in characters, it can be zero to limit
	 *        the width only.
	 * @param text Label displayed text.
	 */
	uilabel(uint16_t id, int top, int left, int width, int height,
			const char *text);
	~uilabel();

	/** Set or update displayed text.
	 *
	 * @param text Text to be displayed.
	 * @return True on success, false on allocation error.
	 */
	bool_t set_text(const char *text);

	virtual void repaint();

private:
	/** Word wrap the text when width and height are specified.
	 *
	 * @param max_lines Maximum allowed lines.
	 * @return Effective maximum text width in charaters.
	 */
	int wrap_text(int max_lines);

	/** Label displayed text. */
	char *m_text;
	/** Pointers to first character of each line of wrapped text. */
	char **m_lines;
	/** Line count of wrapped text. */
	int m_lines_count;
	/** Width of longest line of text in characters when not autosized. */
	int m_effective_width;
	/** Flag to track multiline or single line label. */
	bool_t m_multiline;
	/** Flag to track autosize. */
	bool_t m_autosize;
};

#endif
