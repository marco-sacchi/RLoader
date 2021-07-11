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

bool_t bitmap::load(const char *filename) {
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

	if (m_info.bpp != 4 || m_info.compression != 0) {
		fclose(fp);
		m_error = BMP_ERR_INVALID_FORMAT;
		return FALSE;
	}

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
		image_ptr -= (uint16_t)m_stride;
	}

	fclose(fp);

	return TRUE;
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
