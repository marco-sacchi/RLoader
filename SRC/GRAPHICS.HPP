#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "types.hpp"
#include "vga.hpp"
#include "ansipic.hpp"
#include "bitmap.hpp"

/** Graphics utility methods. */
class graphics {
public:
	/** Convert unsigned 32-bit integer to rgb triplet.
	 *
	 * @param value Integer value.
	 * @param color Valorized with rgb values.
	 */
	static void uint32_to_color(uint32_t value, vga::color_t *color);
	/** Linear interpolate two colors.
	 *
	 * @param from Source color.
	 * @param to Destination color.
	 * @param t Interpolation factor in range [0.0, 1.0].
	 * @param result Valorized with resulting interpolated color.
	 */
	static void lerp_color(const vga::color_t from, const vga::color_t to,
						  float t, vga::color_t *result);
	/** Set palette contained on the bitmap on graphics card.
	 *
	 * @param bitmap Bitmap from which read the palette to set.
	 */
	static void set_palette(bitmap *bmp);
	/** Set palette contained in the bitmap on 16 color modes.
	 *
	 * @param bitmap Bitmap from which read the palette to set.
	 * @param index_offset Offset to apply to palette index when color is set.
	 */
	static void set_ega_palette(bitmap *bmp, int index_offset = 0);
	/** Fade-in palette contained in the bitmap on 16 color modes.
	 *
	 * @param bmp Bitmap from which read the palette to fade in.
	 * @param color Color from which fade.
	 * @param duration Fading effect duration in milliseconds.
	 */
	static void ega_fade_in(bitmap *bmp, const vga::color_t color,
							int duration);
	/** Fade-out palette contained in the bitmap on 16 color modes.
	 *
	 * @param bmp Bitmap from which read the palette to fade out.
	 * @param color Color to which fade.
	 * @param duration Fading effect duration in milliseconds.
	 */
	static void ega_fade_out(bitmap *bmp, const vga::color_t color,
							 int duration);
	/** Debug method to show palette on the firse row of pixels. */
	static void show_palette();

	/** Draw bitmap on specified position.
	 *
	 * @note This method is a specific implementation for 16 colors
	 * planar modes and floor the x position the nearest multiple of 8.
	 * @param bmp Bitmap to draw to the video memory.
	 * @param x Horizontal position of top-left image corner.
	 * @param y Vertical position of top-left image corner.
	 */
	static void draw(bitmap *bmp, uint16_t x, uint16_t y);

	/** Draw ANSI/ASCII picture on specified position.
	 *
	 * @param pic Picture to draw to the video memory.
	 * @param row Zero-based character row of top-left picture corner.
	 * @param column Zero-based character column of top-left picture corner.
	 */
	static void draw(ansi_picture *pic, uint8_t row, uint8_t column);

	/** Draw filled rectangle using text BIOS services.
	 *
	 * @param rect Rectangle to fill.
	 * @param attrs Attrbutes of the drawed characters.
	 */
	static void draw_filled_rect(vga::text_rect_t *rect, uint8_t attrs);

	/** Draw frame using text BIOS services.
	 *
	 * @deprecated use uigraph::draw_frame
	 * @param rect Rectangle into which draw the frame.
	 * @param attrs Attrbutes of the drawed characters.
	 */
	static void draw_frame(vga::text_rect_t *rect, uint8_t attrs);

	/** Draw filled frame using text BIOS services.
	 *
     * @deprecated use uigraph::draw_filled_frame
	 * @param rect Rectangle into which draw the frame.
	 * @param attrs Attrbutes of the drawed characters.
	 */
	static void draw_filled_frame(vga::text_rect_t *rect, uint8_t attrs);

	static void draw_outline(vga::text_rect_t *rect, uint8_t color);
	static void draw_focus_outline(vga::text_rect_t *rect, uint8_t color);

	/** Draw scroll bar using text BIOS services.
	 *
	 * @param rect Reference rectangle, the rendered scroll bar is of the
	 *             same height and positioned 1 character after the right
	 *             boundary of the rect.
	 * @param attrs Attrbutes of the drawed characters.
	 * @param count Number of entries represented by the scrollbar.
	 * @param index Current entry that controls position of scrollbar thumb.
	 */
	static void draw_scroll_bar(vga::text_rect_t *rect, uint8_t attrs,
								int count, int index);

	/** Apply bold effect ORing the pixels of already rendered characters.
	 *
	 * @param row Row of first text character cell.
	 * @param column Column of first text character cell.
	 * @param chars Number of characters for which apply the effect.
	 */
	static void bold_effect(uint8_t row, uint8_t column, uint8_t chars);
	/** Apply italic effect shifting the pixels of already rendered characters.
	 *
	 * @param row Row of first text character cell.
	 * @param column Column of first text character cell.
	 * @param chars Number of characters for which apply the effect.
	 */
	static void italic_effect(uint8_t row, uint8_t column, uint8_t chars);

private:
	/** Linear interpolate palette in the bitmap aganist specified color.
	 *
	 * @param bmp Bitmap from which read the palette to interpolate.
	 * @param to_color Color into which interpolate.
	 * @param t Interpolation factor in range [0.0, 1.0].
	 */
	static void lerp_ega_palette(bitmap *bmp, const vga::color_t to_color, float t);

	static void draw_horiz_line(int x, int y, int length, uint8_t color);

	static void draw_vert_block(int x, int y, int length, uint8_t color, uint8_t bits);

	static void or_horiz_line(int x, int y, int length, uint8_t mask, uint8_t bits);

	static void or_vert_block(int x, int y, int length, uint8_t mask, uint8_t bits);
};

#endif
