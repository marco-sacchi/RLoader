#include <malloc.h>
#include <mem.h>
#include <stdio.h>

#include "ansipic.hpp"

ansi_picture::ansi_picture() {
	m_data = NULL;
	m_width = m_height = m_stride = 0;
	m_error = ANSIPIC_ERR_NONE;
}

ansi_picture::~ansi_picture() {
	this->unload();
}

int ansi_picture::get_last_error() {
	return m_error;
}

uint16_t ansi_picture::get_width() {
	return m_width;
}

uint16_t ansi_picture::get_height() {
	return m_height;
}

const uint8_t *ansi_picture::get_data() {
	return m_data;
}

uint32_t ansi_picture::get_stride() {
	return m_stride;
}

bool_t ansi_picture::load(const char *filename, uint8_t width) {
	this->unload();

	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		m_error = ANSIPIC_ERR_NOTFOUND;
		return FALSE;
	}

	fseek(fp, 0, SEEK_END);
	int file_size = (int)ftell(fp);

	if (((file_size >> 1) % width) != 0) {
		fclose(fp);
		m_error = ANSIPIC_ERR_INVALID;
		return FALSE;
	}

	m_width = width;
	m_stride = (uint16_t)width << 1;
	m_height = file_size / m_stride;

	m_data = (uint8_t *)malloc(file_size);
	if (m_data == NULL) {
		m_error = ANSIPIC_ERR_NOMEM;
		return FALSE;
	}

	rewind(fp);
	fread(m_data, m_stride, m_height, fp);
	fclose(fp);

	m_error = ANSIPIC_ERR_NONE;
	return TRUE;
}

void ansi_picture::unload() {
	if (m_data != NULL)
		free(m_data);

	m_error = ANSIPIC_ERR_NONE;
	m_stride = 0;
	m_data = NULL;
}
