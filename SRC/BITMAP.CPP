#include <malloc.h>
#include <mem.h>
#include <stdio.h>

#include "bitmap.hpp"

bitmap::bitmap() {
	memset(&m_info, 0, sizeof(bitmap::info_header_t));
	m_error = BMP_ERR_NONE;
	m_stride = 0;
	m_palette = NULL;
	m_image = NULL;
}

bitmap::~bitmap() {
	this->unload();
}

int bitmap::get_last_error() {
	return m_error;
}

uint32_t bitmap::get_width() {
	return m_info.width;
}

uint32_t bitmap::get_height() {
	return m_info.height;
}

uint32_t *bitmap::get_palette(uint32_t *colors) {
	*colors = m_info.colors;
	return m_palette;
}

void *bitmap::get_image() {
	return m_image;
}

uint32_t bitmap::get_stride() {
	return m_stride;
}

bool_t bitmap::load(const char *filename, int color_offset) {
	this->unload();

	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		m_error = BMP_ERR_NOTFOUND;
		return FALSE;
	}

	bitmap::file_header_t file_header;
	fread(&file_header, sizeof(bitmap::file_header_t), 1, fp);

	if (file_header.type != BMP_TYPE) {
		fclose(fp);
		m_error = BMP_ERR_INVALID;
		return FALSE;
	}

	fread(&(m_info.size), 4, 1, fp);
	fread(((uint8_t *)&m_info + 4), (size_t)m_info.size - 4, 1, fp);

	if (m_info.colors > 0) {
		m_palette = (uint32_t *)malloc((size_t)m_info.colors * 4);
		if (m_palette == NULL) {
			fclose(fp);
			this->unload();
			m_error = BMP_ERR_NOMEM;
			return FALSE;
		}

		fread(m_palette, (size_t)m_info.colors * 4, 1, fp);
	}

	fseek(fp, file_header.offset, SEEK_SET);
	m_stride = (((m_info.bpp * m_info.width) >> 3) + 3) & ~0x03;

	m_image = malloc((size_t)(m_stride * m_info.height));
	if (m_image == NULL) {
		fclose(fp);
		this->unload();
		m_error = BMP_ERR_NOMEM;
		return FALSE;
	}

	uint32_t i;
	uint8_t *image_ptr = (uint8_t *)m_image;
	for (i = 0; i < m_info.height - 1; ++i)
		image_ptr += (uint16_t)m_stride;

	for (i = 0; i < m_info.height; ++i) {
		fread(image_ptr, (size_t)m_stride, 1, fp);
		this->encode_planar_4bits(image_ptr, color_offset);
		image_ptr -= (uint16_t)m_stride;
	}

	fclose(fp);

	return TRUE;
}

void bitmap::encode_planar_4bits(const uint8_t *row, int color_offset) {
	uint8_t *pixel;
	uint8_t *encoded = m_encode_buffer;

	pixel = (uint8_t *)row;
	for (int32_t x = 0; x < m_stride; ++x)
		*pixel++ = (uint8_t)(((*pixel & 0x0f) + color_offset) |
			(*pixel & 0xf0) + (color_offset << 4));

	for (uint16_t plane = 0; plane < 4; ++plane) {
		pixel = (uint8_t *)row;
		for (int32_t i = 0; i < m_info.width; i += 8) {
			*encoded = 0;
			for (uint16_t j = 0; j < 8; ++j)
				*encoded |=
				(((pixel[j >> 1]      /* select byte */
				>> (4 - 4 * (j & 1))  /* select 4-bit pixel */
				) >> plane) & 1)      /* get pixel bit by plane */
				<< (7 - j);           /* shift to set correct bit */

			++encoded;
			pixel += (8 * m_info.bpp) >> 3;
		}
	}

	memcpy((void *)row, m_encode_buffer, (size_t)m_stride);
}

void bitmap::unload() {
	if (m_palette != NULL)
		free(m_palette);

	if (m_image != NULL)
		free(m_image);

	memset(&m_info, 0x00, sizeof(bitmap::info_header_t));
	m_error = BMP_ERR_NONE;
	m_stride = 0;
	m_palette = NULL;
	m_image = NULL;
}
