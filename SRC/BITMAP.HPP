#ifndef BITMAP_HPP
#define BITMAP_HPP

#include "types.hpp"

#define BMP_TYPE	0x4d42

#define BMP_ERR_NONE			0	/* No error. */
#define BMP_ERR_NOTFOUND		-1	/* File not found. */
#define BMP_ERR_INVALID     	-2	/* Invalid/corrupted bitmap file. */
#define BMP_ERR_NOMEM			-3	/* Not enough memory to allocate bitmap. */
#define BMP_ERR_INVALID_FORMAT	-4	/* Non 4-bit/pixel uncompressed bitmap.*/

#define BMP_CMP_NONE			0	/* None */
#define BMP_CMP_RLE8			1	/* RLE 8-bit/pixel, used only with 8-bit/pixel bitmaps */
#define BMP_CMP_RLE4			2	/* RLE 4-bit/pixel, used only with 4-bit/pixel bitmaps */
#define BMP_CMP_BITFIELDS		3	/* Huffman 1D, v2 RGB bit field masks, v3 RGBA */
#define BMP_CMP_JPEG			4	/* RLE-24, JPEG image for printing */
#define BMP_CMP_PNG				5	/* RLE-24, PNG image for printing */
#define BMP_CMP_ALPHABITFIELDS	6	/* RGBA bit field masks only Windows CE 5.0 with .NET 4.0 or later */
#define BMP_CMP_CMYK			11	/* None, only Windows Metafile CMYK */
#define BMP_CMP_CMYKRLE8		12	/* RLE-8, only Windows Metafile CMYK */
#define BMP_CMP_CMYKRLE4		13	/* RLE-4, only Windows Metafile CMYK */

#pragma pack(push, 1);

/** Object to load uncompressed bitmap files.
 *
 * @note This is a simplified implementation, not all formats are supported.
 */
class bitmap {

private:
	/** Bitmap file header. */
	typedef struct {
		/** Type, must be BMP_TYPE. */
		uint16_t type;
		/** Size of this header. */
		uint32_t size;
		/** Reserved. */
		uint16_t reserved, reserved2;
		/** Offset of image data from beginning of file. */
		uint32_t offset;
	} file_header_t;

	/** Bitmap info header. */
	typedef struct {
		/** Size of this header. */
		uint32_t size;
		/** Bitmap width in pixels. */
		int32_t width;
		/** Bitmap height in pixels. */
		int32_t height;
		/** Number of color planes (must be 1). */
		uint16_t planes;
		/** Number of bits per pixel. */
		uint16_t bpp;
		/** Compression method being used. */
		uint32_t compression;
		/** Size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps. */
		uint32_t imgsize;
		/** Horizontal resolution of the image (pixel per meter). */
		int32_t horiz_res;
		/** Vertical resolution of the image (pixel per meter). */
		int32_t vert_res;
		/** Number of colors in the color palette, or 0 to default to 2n. */
		uint32_t colors;
		/** Number of important colors used. */
		uint32_t important_colors;
	} info_header_t;

	/** Last error value, see BMP_ERR_* defines. */
	int m_error;
	/** Bitmap info header. */
	info_header_t m_info;
	/** Image data stride in *bytes*. */
	int32_t m_stride;
	/** Palette entries as packed 32-bit unsigned int 0x00BBGGRR. */
	uint32_t *m_palette;
	/** Pointer to image. */
	void *m_image;
	/** Unload bitmap resources. */
	void unload();

public:
	bitmap();
	~bitmap();

	/** Get last error code after call to load method.
	 *
	 * @return Error code as BMP_ERR_* define.
	 */
	int get_last_error();

	/** Get bitmap width.
	 *
	 * @return Width in pixels.
	 */
	uint32_t get_width();
	/** Get bitmap height.
	 *
	 * @return Height in pixels.
	 */
	uint32_t get_height();

	/** Get pointer to palette data.
	 *
	 * @param colors Set to number of colors in the palette.
	 * @return Pointer to first palette entry.
	 */
	uint32_t *get_palette(uint32_t *colors);
	/** Get raw image data.
	 *
	 * @return Image data pointer.
	 */
	void *get_image();
	/** Get stride between image rows.
	 *
	 * @return Number of *bytes* to skip to the next row.
	 */
	uint32_t get_stride();

	/** Load bitmap in memory.
	 *
	 * @param filename Fully-qualified path of bitmap file with extension.
	 * @return TRUE on success, FALSE otherwise.
	 */
	bool_t load(const char *filename);
};

#pragma pack(pop);

#endif
