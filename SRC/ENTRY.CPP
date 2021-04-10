#include <alloc.h>
#include <string.h>

#include "string.hpp"
#include "math.hpp"
#include "entry.hpp"

list_entry::list_entry() {
	m_path = m_executable = m_setup = m_title = NULL;
	m_cycles = 0;
}

list_entry::~list_entry() {
	if (m_path != NULL)
		free(m_path);

	if (m_executable != NULL)
		free(m_executable);

	if (m_setup != NULL)
		free(m_setup);

	if (m_title != NULL)
		free(m_title);

	m_path = m_executable = m_setup = m_title = NULL;
	m_cycles = 0;
}

bool_t list_entry::parse(const char *line) {
	if (line == NULL || *line == NULL)
		return FALSE;

	const char *src_ptr = line;
	char buffer[64];

	int length = this->parse_field(src_ptr, buffer);
	if (length < 0)
		return FALSE;

	m_path = (char *)malloc(strlen(buffer) + 1);
	strcpy(m_path, buffer);

	src_ptr += length;
	length = this->parse_field(src_ptr, buffer);
	if (length < 0)
		return FALSE;

	m_executable = (char *)malloc(strlen(buffer) + 1);
	strcpy(m_executable, buffer);

	src_ptr += length;
	length = this->parse_field(src_ptr, buffer);
	if (length < 0)
		return FALSE;

	m_setup = (char *)malloc(strlen(buffer) + 1);
	strcpy(m_setup, buffer);

	src_ptr += length;
	length = this->parse_cycles(src_ptr, &m_cycles);
	if (length < 0)
		return FALSE;

	src_ptr += length;

	// Search for title string.
	while (string::is_white_space(*src_ptr))
		++src_ptr;

	length = strlen(src_ptr);
	char *new_line = strpbrk(src_ptr, "\n\r");
	if (new_line != NULL) {
		length = min((int)(new_line - (char *)src_ptr), length);
		*new_line = NULL;
	}

	new_line = (char *)src_ptr + length - 1;
	while (string::is_white_space(*new_line) && length > 0) {
		*new_line-- = NULL;
		--length;
	}

	if (length < 1)
		return FALSE;

	m_title = (char *)malloc(length + 1);
	strcpy(m_title, src_ptr);

	char *chr = m_title;
	while (*chr != NULL) {
		if (string::is_white_space(*chr))
			*chr = ' ';

		++chr;
	}

	return TRUE;
}

int list_entry::parse_field(const char *source, const char *dest) {
	const char *src_ptr = source;
	while (string::is_white_space(*src_ptr)) {
		if (*src_ptr == NULL)
			return -1;

		++src_ptr;
	}

	char *dest_ptr = (char *)dest;
	while (!string::is_white_space(*src_ptr)) {
		if (*src_ptr == NULL)
			return -1;

		*dest_ptr++ = *src_ptr++;
	}

	*dest_ptr = NULL;
	return (int)(src_ptr - source);
}

int list_entry::parse_cycles(const char *source, uint16_t *dest) {
	const char *src_ptr = source;
	while (string::is_white_space(*src_ptr)) {
		if (*src_ptr == NULL)
			return -1;

		++src_ptr;
	}

	int digit_count = 0;
	uint16_t digits[5];
	while (!string::is_white_space(*src_ptr)) {
		if (digit_count > 4 || *src_ptr == NULL ||
			*src_ptr < '0' || *src_ptr > '9')
			return -1;

		digits[digit_count++] = *src_ptr++ - '0';
	}

	*dest = NULL;
	uint16_t multiplier = 1;
	for (int i = digit_count - 1; i >= 0; --i) {
		*dest += digits[i] * multiplier;
		multiplier *= 10;
	}

	return (int)(src_ptr - source);
}

bool_t list_entry::match(const char *search) {
	if (string::icase_match(this->get_folder(), search))
		return TRUE;

	if (string::icase_match(m_title, search))
		return TRUE;

	return FALSE;
}

const char *list_entry::get_folder() const {
	char *last_dir = max(strrchr(m_path, '\\'), strrchr(m_path, '/'));
	if (last_dir == NULL)
		last_dir = (char *)m_path;
	else
		++last_dir;

	return last_dir;
}
