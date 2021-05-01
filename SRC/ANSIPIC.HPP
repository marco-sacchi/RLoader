#ifndef ANSIPIC_HPP
#define ANSIPIC_HPP

#include "types.hpp"

#define ANSIPIC_ERR_NONE		0	/* No error. */
#define ANSIPIC_ERR_NOTFOUND	-1	/* File not found. */
#define ANSIPIC_ERR_INVALID		-2	/* Invalid/corrupted file. */
#define ANSIPIC_ERR_NOMEM		-3	/* Not enough memory to allocate picture. */

/** Object to load ANSI/ASCII pictures.
 *
 * @note The supported format is binary 2-bytes per character.
 */
class ansi_picture {

private:
	/** Last error value, see ANSIPIC_ERR_* defines. */
	int m_error;

	/** Pointer to picture data. */
	uint8_t *m_data;
	/** Width in characters. */
	uint8_t m_width;
	/** Height in characters. */
	uint8_t m_height;
	/** Stride in *bytes* between rows. */
	uint16_t m_stride;

	/** Unload picture resources. */
	void unload();

public:
	ansi_picture();
	~ansi_picture();

	/** Get last error code after call to load method.
	 *
	 * @return Error code as ANSIPIC_ERR_* define.
	 */
	int get_last_error();

	/** Get picture width.
	 *
	 * @return Width in pixels.
	 */
	uint16_t get_width();
	/** Get picture height.
	 *
	 * @return Height in pixels.
	 */
	uint16_t get_height();

	/** Get raw image data.
	 *
	 * @return Image data pointer.
	 */
	const uint8_t *get_data();
	/** Get stride between image rows.
	 *
	 * @return Number of *bytes* to skip to the next row.
	 */
	uint32_t get_stride();

	/** Load picture in memory.
	 *
	 * @param filename Fully-qualified path of ANSI file with extension.
	 * @param width Picture width in characters.
	 * @return TRUE on success, FALSE otherwise.
	 */
	bool_t load(const char *filename, uint8_t width);
};

#endif
