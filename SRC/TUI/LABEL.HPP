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
	 * @param caption Label displayed text.
	 */
	uilabel(uint16_t id, int top, int left, const char *caption);
	~uilabel();

	virtual void repaint();

private:
	/** Label displayed text. */
	char *m_text;
};

#endif
