#ifndef STRING_HPP
#define STRING_HPP

#include "types.hpp"

/** String utilites methods. */
class string {
public:
	/** Test for whitespace character.
	 *
	 * @param c Character to test.
	 * @return TRUE if character is whitespace, FALSE otherwise.
	 */
	static bool_t is_white_space(char c);
	/** Test for punctuation character.
	 *
	 * @param c Character to test.
	 * @return TRUE if character is punctuation, FALSE otherwise.
	 */
	static bool_t is_punctuation(char c);
	/** Test for digit character.
	 *
	 * @param c Character to test.
	 * @return TRUE if character is digit, FALSE otherwise.
	 */
	static bool_t is_digit(char c);
	/** Test for letter character.
	 *
	 * @note This method do not support extended ASCII characters.
	 * @param c Character to test.
	 * @return TRUE if character is letter, FALSE otherwise.
	 */
	static bool_t is_letter(char c);
	/** Compare two character case-insensitively.
	 *
	 * @note This method do not support extended ASCII characters.
	 * @param a First character.
	 * @param b Second character.
	 * @return TRUE if characters match, FALSE otherwise.
	 */
	static bool_t icase_compare(char a, char b);
	/** Test for empty string.
	 *
	 * @param str String to test.
	 * @return TRUE if string is made only of whitespace characters.
	 */
	static bool_t is_empty(const char *str);
	/** Test for substring match case-insensitively.
	 *
	 * @param str String to search in.
	 * @param search String to be searched.
	 * @return Pointer to first match, NULL if no match are found.
	 */
	static char *icase_match(const char *str, const char *search);
	/** Return all case-insesitive matches of a substring.
	 *
	 * @note matches must be already allocated.
	 * @param str String to search in.
	 * @param search String to be searched.
	 * @param matches Valorized with pointers to first character of matches.
	 * @return Number of matches.
	 */
	static int icase_match_all(const char *str, const char *search, const char **matches);
};

#endif