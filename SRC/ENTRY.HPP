#ifndef ENTRY_HPP
#define ENTRY_HPP

#include "types.hpp"

/** List entry object. */
class list_entry {
public:
	list_entry();
	~list_entry();

	/** Get entry index.
	 *
	 * @return Zero-based entry index into the list.
	 */
	uint16_t get_index() const { return m_index; }
	/** Set entry index.
	 *
	 * @param index Zero-based entry index into the list.
	 */
	void set_index(uint16_t index) { m_index = index; }

	/** Parse entry from list file.
	 *
	 * @param line Line of list file.
	 * @return TRUE on successful parsing, FALSE otherwise.
	 */
	bool_t parse(const char *line);
	/** Test if this entry matches the search string.
	 *
	 * @param search Search string.
	 * @return TRUE if entry matches, FALSE otherwise.
	 */
	bool_t match(const char *search);
	/** Get entry folder.
	 *
	 * @return The inner folder of entry path.
	 */
	const char *get_folder() const;

	/** Get entry path.
	 *
	 * @return Entry fully-qualified path.
	 */
	const char *get_path() const { return m_path; }
	/** Get entry executable.
	 *
	 * @return Executable filename with extension.
	 */
	const char *get_executable() const { return m_executable; }
	/** Get entry setup program.
	 *
	 * @return Setup program executable filename with extension.
	 */
	const char *get_setup() const { return m_setup; }
	/** Entry title.
	 *
	 * @return Entry descriptive title.
	 */
	const char *get_title() const { return m_title; }
	/** Get cycles required for emulation.
	 *
	 * @return The number of cycles to set when this program run on DOSBox.
	 */
	uint16_t get_cycles() const { return m_cycles; }
	/** Set cycles required for emulation.
	 *
	 * @param cycles The number of cycles to set when this program
	 *               run on DOSBox.
	 */
	void set_cycles(uint16_t cycles) { m_cycles = cycles; }

private:
	/** Parse field.
	 *
	 * @param source List file line or part of it.
	 * @param dest Valorized with readed field trimmed from whitespaces.
	 * @return Number of characters parsed (whitespace included), -1 on error.
	 */
	int parse_field(const char *source, const char *dest);
	/** Parse cycles field as integer.
	 *
	 * @param source List file line or part of it.
	 * @param dest Valorized with number of cycles.
	 * @return Number of characters parsed (whitespace included), -1 on error.
	 */
	int parse_cycles(const char *source, uint16_t *dest);

	/** Zero-based entry index into the list. */
	uint16_t m_index;
	/** Entry path, executable and setup program. */
	char *m_path, *m_executable, *m_setup;
	/** Number of cycles to set when run entry on DOSBox. */
	uint16_t m_cycles;
	/* Entry descriptive title. */
	char *m_title;
};

#endif