#include <string.h>

#include "string.hpp"

bool_t string::is_eol(char c) {
	return (c == '\r' || c == '\n') ? TRUE : FALSE;
}

bool_t string::is_white_space(char c) {
	return (c == ' ' || c == '\t') ? TRUE : FALSE;
}

bool_t string::is_punctuation(char c) {
	return ((c >= 0x21 && c <= 0x2f) ||
			(c >= 0x3a && c <= 0x40) ||
			(c >= 0x5b && c <= 0x60) ||
			(c >= 0x7b && c <= 0x7e)) ? TRUE : FALSE;
}

bool_t string::is_digit(char c) {
	return (c >= '0' && c <= '9') ? TRUE : FALSE;
}

bool_t string::is_letter(char c) {
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) ? TRUE : FALSE;
}

bool_t string::icase_compare(char a, char b) {
	if (a == b) return TRUE;

	if (a >= 'a' && a <= 'z') a -= 32;
	if (b >= 'a' && b <= 'z') b -= 32;

	return (a == b) ? TRUE : FALSE;
}

bool_t string::is_empty(const char *str) {
	const char *ptr = str;
	while (string::is_white_space(*ptr))
		++ptr;

	return (*ptr == NULL) ? TRUE : FALSE;
}

int string::icase_index_of(const char *str, char search) {
	char search_str[2] = { search, NULL };
	const char *ptr = string::icase_match(str, search_str);
	if (ptr == NULL)
		return -1;

	return (int)(ptr - str);
}

char string::last_char(const char *str) {
	int length = strlen(str);
	if (length == 0)
		return '\0';

	return *(str + length - 1);
}

char *string::icase_match(const char *str, const char *search) {
	const char *source = str;
	const char *search_ptr = search;

	while (*source != NULL) {
		while (*source != NULL && !string::icase_compare(*source, *search))
			source++;

		if (*source == NULL)
			return NULL;

		while (*source != NULL && *search_ptr != NULL &&
			   string::icase_compare(*source, *search_ptr)) {
			source++;
			search_ptr++;
		}

		if (*search_ptr == NULL)
			return (char *)(source - strlen(search));

		search_ptr = search;
	}

	return NULL;
}

int string::icase_match_all(const char *str, const char *search, const char **matches) {
	int match_count = 0;
	const char *end = str + strlen(str) - 1;
	size_t search_len = strlen(search);
	if (end < str || search_len == 0)
		return 0;

	char *match = (char *)str;
	while (match <= end &&
		   (match = string::icase_match(match, search)) != NULL) {
		if (*match == NULL)
			break;

		matches[match_count++] = match;
		match += search_len;
	}

	return match_count;
}