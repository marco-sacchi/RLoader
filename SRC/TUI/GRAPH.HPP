#ifndef TUI_GRAPH_HPP
#define TUI_GRAPH_HPP

#include "graphics.hpp"
#include "tui/theme.hpp"

#define FRAME_CHARS		8

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

	/** Modify characters generator bitmap to render customized frame.
	 *
	 * @return True on success, false on unrecognized font height.
	 */
	static bool_t setup_frame_chars();
	/** Restore modified charaters generator bitmap. */
	static void restore_chars();

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

	/** Draw frame accounting for customized characters.
	 *
	 * @deprecated use uigraph::draw_frame
	 * @param rect Rectangle into which draw the frame.
	 * @param attrs Attrbutes of the drawed characters.
	 */
	static void draw_frame(vga::text_rect_t *rect, uint8_t attrs);
	/** Draw filled frame accounting for customized characters.
	 *
	 * @deprecated use uigraph::draw_filled_frame
	 * @param rect Rectangle into which draw the frame.
	 * @param attrs Attrbutes of the drawed characters.
	 */
	static void draw_filled_frame(vga::text_rect_t *rect, uint8_t attrs);

	/** Highlight (underline in graphic mode) characters.
	 *
	 * @param row Zero-based text row.
	 * @param col Zero-based text column.
	 * @param attrs Painting color attributes of text to hightlight.
	 */
	static void highlight_chars(uint8_t row, uint8_t col, uint8_t attrs,
								uint16_t count);

	/** Reverse item colors.
	 *
	 * @param row Zero-based text row.
	 * @param col Zero-based text column.
	 * @param attrs Painting color attributes.
	 * @param count Item width as characters count.
	 */
	static void reverse_item(uint8_t row, uint8_t col, uint8_t attrs,
							 uint16_t count);

	/** Apply focus style to an item.
	 *
	 * @param row Zero-based text row.
	 * @param col Zero-based text column.
	 * @param attrs Painting color attributes.
	 * @param count Item width as characters count.
	 */
	static void apply_focus(uint8_t row, uint8_t col, uint8_t attrs,
							uint8_t count);

	/** Apply focus style to specified rect.
	 *
	 * @param rect Rect for which apply style.
	 * @param attrs Painting color attributes of control.
	 * @param style Focus style.
	 */
	static void apply_focus(vga::text_rect_t *rect, uint8_t attrs,
							focus_style style);

	/** Remove applied focus style to specified rect.
	 *
	 * @note Only for graphics UI, no effect on text-mode UI.
	 * @param rect Rect for which apply style.
	 * @param attrs Painting color attributes of control.
	 * @param style Previously specified focus style.
	 */
	static void remove_focus(vga::text_rect_t *rect, uint8_t attrs,
							 focus_style style);

	/** Apply active style to an item.
	 *
	 * @param row Zero-based text row.
	 * @param col Zero-based text column.
	 * @param attrs Painting color attributes.
	 * @param count Item width as characters count.
	 */
	static void apply_active(uint8_t row, uint8_t col, uint8_t attrs,
							 uint8_t count);

	/** Apply active style to specified rect.
	 *
	 * @param rect Rect for which apply style.
	 * @param attr Painting color attributes of control.
	 */
	static void apply_active(vga::text_rect_t *rect, uint8_t attr);

	/** Render a shadow around specified rectangle clipping it if necessary.
	 *
	 * @param rect Reference rectangle.
	 */
	static void draw_shadow(vga::text_rect_t *rect);

private:
	/** True when character generator is customized, false otherwise. */
	static bool_t custom_frame;
	/** Original character generator bitmap data. */
	static uint8_t previous_chars[FRAME_CHARS * 16];
	/** Original character generator char height. */
	static int previous_char_height;
	/** Frame charaters indices. */
	static uint8_t uigraph::frame_chars[FRAME_CHARS];
};

#endif
