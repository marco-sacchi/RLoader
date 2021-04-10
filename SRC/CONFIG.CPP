#include <stdio.h>
#include <string.h>

#include "string.hpp"
#include "config.hpp"

config::config() {
	for (int i = 0; i < 'Z' - 'A' + 1; ++i)
		m_drives_mapping[i] = 'A' + i;
}

config::~config() {

}

bool_t config::load(const char *filename) {
	FILE *fp = fopen(filename, "rt");
	if (fp == NULL)
		return FALSE;

	char line[80], *key, *value;
	bool_t result = TRUE;
	int line_count = 1;
	while (fgets(line, 80, fp) != NULL) {
		if (*line == '#') {
			++line_count;
			continue;
		}

		if (this->parse_line(line, &key, &value) == FALSE) {
			result = FALSE;
			break;
		}

		if (key == NULL && value == NULL) {
			++line_count;
			continue;
		}

		strlwr(key);

		if (strcmp(key, "ui-bg-color") == 0) {
			strlwr(value);
			if (this->parse_color(value, &m_bg_color) == FALSE) {
				result = FALSE;
				break;
			}
		} else if (strcmp(key, "ui-fg-color") == 0) {
			strlwr(value);
			if (this->parse_color(value, &m_fg_color) == FALSE) {
				result = FALSE;
				break;
			}
		} else if (string::is_letter(*key) && strlen(key) == 1) {
			if (!string::is_letter(*value) || strlen(value) != 1) {
				result = FALSE;
				break;
			}

			strupr(key);
			strupr(value);
			if (*key != *value)
				m_drives_mapping[*key - 'A'] = *value;
		} else {
			result = FALSE;
			break;
		}
		++line_count;
	}

	if (result == FALSE)
		printf("%s: Error parsing line %d\n", filename, line_count);

	fclose(fp);
	return result;
}

bool_t config::parse_line(char *line, char **key, char **value) {
	*key = NULL;
	*value = NULL;

	char *chr = line;

	// Strip line terminators and trailing spaces.
	char *new_line = strpbrk(line, "\n\r");
	if (new_line != NULL)
		*new_line = NULL;

	while (string::is_white_space(*(--new_line)) && new_line > line)
		*new_line = NULL;

	// Skip white-spaces before key.
	while (string::is_white_space(*chr) && *chr != NULL)
		++chr;

	// Empty line.
	if (*chr == NULL)
		return TRUE;

	// Set key pointer and go forward until white-space,
	// equal character or end of string.
	*key = chr;

	while (!string::is_white_space(*chr) && *chr != '=' && *chr != NULL)
		++chr;

	if (*chr == NULL)
		return FALSE;

	if (*chr != '=') {
		*chr++ = NULL;
		while (string::is_white_space(*chr) && *chr != NULL)
			++chr;

		if (*chr != '=')
			return FALSE;
	}

	*chr++ = NULL;

	// Skip white-spaces between equal character and start of value.
	while (string::is_white_space(*chr) && *chr != NULL)
		++chr;

	// No value.
	if (*chr == NULL)
		return FALSE;

	// Set value.
	*value = chr;

	return TRUE;
}

bool_t config::parse_color(const char *value, vga::color_t *color) {
	int r, g, b;
	if (sscanf(value, "#%02x%02x%02x", &r, &g, &b) != 3)
		return FALSE;

	(*color)[0] = r;
	(*color)[1] = g;
	(*color)[2] = b;

	return TRUE;
}

char config::get_drive_mapping(char drive_letter) {
	if (!string::is_letter(drive_letter))
		return NULL;

	if (drive_letter >= 'a')
		drive_letter -= 'a';
	else
		drive_letter -= 'A';

	if (drive_letter < 0 || drive_letter > ('Z' - 'A'))
		return NULL;

	return m_drives_mapping[drive_letter];
}
