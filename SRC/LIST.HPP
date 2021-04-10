#ifndef LIST_HPP
#define LIST_HPP

#include <stdio.h>

#include "types.hpp"
#include "entry.hpp"

#define LIST_PATH_SIZE	40

/** List object. */
class list {

public:
	list();
	~list();

	/** Load list file.
	 *
	 * @param path Fully-qualified path with extension.
	 * @return TRUE on success, FALSE otherwise.
	 */
	bool_t load(const char *path);
	/** Release list resources. */
	void unload();

	/** Get list file path.
	 *
	 * @return Fully-qualified list file path.
	 */
	const char *get_path() { return m_list_path; }

	/** Get list entry.
	 *
	 * @param index Entry zero-based index.
	 * @return Entry object at specified index.
	 */
	list_entry *get_entry(int index);
	/** Get list entries.
	 *
	 * @return Pointer to first entry.
	 */
	list_entry *get_list();
	/** Get list entry count.
	 *
	 * @return Number of entries in list.
	 */
	int get_entry_count() { return m_entry_count; };

	/** Filter list by search string.
	 *
	 * @param search Text to search in entries.
	 * @param filtered_count Valorized with number of matching items.
	 * @return Array of pointers to matching entries.
	 */
	list_entry **filter(const char *search, int *filtered_count);

private:
	/** Count entries skipping comments and empty lines.
	 *
	 * @param fp List file pointer.
	 * @return Number of entries.
	 */
	int count_entries(FILE *fp);
	/** Read entries on list file.
	 *
	 * @param fp List file pointer.
	 * @return TRUE on succes, FALSE on parsing error.
	 */
	bool_t read_entries(FILE *fp);
	/** Clear array of pointers to list entries (used for filtering). */
	void clear_entry_list();

	/** Array of list entries. */
	list_entry *m_entries;
	/** Array of pointers to list entries (used for filtering). */
	list_entry **m_entry_list;
	/** List entries count. */
	int m_entry_count;
	/** List path. */
	char m_list_path[LIST_PATH_SIZE];
};

#endif