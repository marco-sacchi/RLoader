#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "types.hpp"
#include "vga.hpp"

/** Configuration file reader and parser. */
class config {
public:
	config();
	~config();
	/** Read specified configuration file.
	 *
	 * @param filename Fully-qualified file path with extension.
	 * @return TRUE on success, FALSE otherwise.
	 */
	bool_t load(const char *filename);
	/** Get background color
	 *
	 * @return Pointer to background color type.
	 */
	const vga::color_t *get_background_color() { return &m_bg_color; }
	/** Get foreground color
	 *
	 * @return Pointer to foreground color type.
	 */
	const vga::color_t *get_foreground_color() { return &m_fg_color; }
	/** Get mapping for specified drive.
	 *
	 * @param drive_letter Drive letter on list file.
	 * @return Mapped drive letter.
	 */
	char get_drive_mapping(char drive_letter);

private:
	/** Parse configuration file line.
	 *
	 * @note Line will be modified setting NULL characters to correctly
	 * terminate key/values strings.
	 * @param line Line to parse.
	 * @param key Pointer to first character of key on passed line.
	 * @param value Pointer to first character of value on passed line.
	 */
	bool_t parse_line(char *line, char **key, char **value);
	/** Parse HTML color to color type.
	 *
	 * @param value String representation in #RRGGBB format
	 * @param color Valorized with parsed values.
	 * @return TRUE on success, FALSE on parsing error.
	 */
	bool_t parse_color(const char *value, vga::color_t *color);

	/** Readed background and foregraound colors. */
	vga::color_t m_bg_color, m_fg_color;
	/** Readed drives mapping. */
	char m_drives_mapping['Z' - 'A' + 1];
};

#endif