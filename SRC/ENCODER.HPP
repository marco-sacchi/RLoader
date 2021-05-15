#ifndef ENCODER_HPP
#define ENCODER_HPP

#include "types.hpp"
#include "bitmap.hpp"

/** Object to encode bitmaps for displaying on 4-bits planar modes.
 *
 * @note This implementation currently support only 4-bpp bitmaps.
 */
class bitmap_encoder {
public:
	/** Create and initialize the encoder.
	 *
	 * @param bmp Bitmap to be encoded.
	 * @param color_offset Offset applied to color indices.
	 * @param callback Callback to be called when encoding is complete.
	 */
	bitmap_encoder(bitmap *bmp, uint8_t color_offset = 0);
	~bitmap_encoder();

	/** Get bitmap encoder is working on.
	 *
	 * @return Pointer to bitmap.
	 */
	bitmap *get_bitmap() { return m_bitmap; }

	/** Do background encoding.
	 *
	 * @return TRUE when image encoding is complete, FALSE otherwise.
	 */
	bool_t task();

private:
	/** Bitmap to be encoded for planar display. */
	bitmap *m_bitmap;
	/** Pointer to image to be encoded in-place. */
	uint8_t *m_image;
	/** Encoding buffer. */
	uint8_t m_encode_buffer[512];
	/** Offset applied to color index. */
	uint8_t m_color_offset;
	/** Plane stride in bytes. */
	uint16_t m_plane_stride;
	/** Pointer to row to be encoded. */
	uint8_t *m_current_ptr;
	/** Number of rows remaining to complete image encoding. */
	int m_remaining_rows;

	/** Encode row to be displayed on a planar 4-bits mode.
	 *
	 * @param row Pointer to raw data image row.
	 */
	void planar_4bits(const uint8_t *row);
};

#endif
