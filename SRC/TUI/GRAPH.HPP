#ifndef TUI_GRAPH_HPP
#define TUI_GRAPH_HPP

#include "graphics.hpp"

/** Utility methods to abstract graphics/text-mode UI elements drawing. */
class uigraph {

public:
	/** Focus graphics style. */
	enum focus_style {
		/** Focus drawed as an outline, only in graphic mode. */
		FOCUS_OUTLINE = 0x01,
		/** Focus drawed as full control surface colors highlight. */
		FOCUS_SURFACE = 0x02
	};

	/** Enable hardware cursor, only when in text-mode.
	 *
	 * @param enable TRUE to enable caret, FALSE otherwise.
	 */
	static void enable_caret(bool_t enable);

	/** Draw accelerator character.
	 *
	 * @param row Zero-based text row.
	 * @param col Zero-based text column.
	 * @param attrs Painting color attributes of text for which hightlight
	 *              accelerator character.
	 */
	static void draw_accelerator(uint8_t row, uint8_t col, uint8_t attrs);

	/** Highlight (underline in graphic mode) characters.
	 *
	 * @param row Zero-based text row.
	 * @param col Zero-based text column.
	 * @param attrs Painting color attributes of text to hightlight.
	 */
	static void highlight_chars(uint8_t row, uint8_t col, uint8_t attrs,
								uint16_t count);

	/** Apply active style to an item.
	 *
	 * @param row Zero-based text row.
	 * @param col Zero-based text column.
	 * @param attrs Painting color attributes.
	 * @param count Item width as characters count.
	 */
	static void draw_active_item(uint8_t row, uint8_t col, uint8_t attrs,
								 uint8_t count);

	/** Reverse item colors.
	 *
	 * @param row Zero-based text row.
	 * @param col Zero-based text column.
	 * @param attrs Painting color attributes.
	 * @param count Item width as characters count.
	 */
	static void reverse_item(uint8_t row, uint8_t col, uint8_t attrs,
							 uint16_t count);

	/** Apply focus style to specified rect.
	 *
	 * @param rect Rect for which apply style.
	 * @param attr Painting color attributes of control.
	 * @param style Focus style.
	 */
	static void apply_focus(vga::text_rect_t *rect, uint8_t attr,
							focus_style style);
	/** Remove applied focus style to specified rect.
	 *
	 * @param rect Rect for which apply style.
	 * @param attr Painting color attributes of control.
	 * @param style Previously specified focus style.
	 */
	static void remove_focus(vga::text_rect_t *rect, uint8_t attr,
							 focus_style style);

};

#endif
