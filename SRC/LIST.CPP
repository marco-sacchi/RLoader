#include <mem.h>
#include <alloc.h>
#include <string.h>

#include "list.hpp"
#include "string.hpp"

#define LINE_BUFFER_SIZE	128

list::list() {
	m_entries = NULL;
	m_entry_list = NULL;
	m_entry_count = 0;
}

list::~list() {
	this->unload();
}

bool_t list::load(const char *name) {
	char path[LIST_PATH_SIZE];
	strcpy(path, "lists/");
	strcat(path, name);
	strcpy(m_list_path, path);
	strcat(path, "/list.txt");

	FILE *fp = fopen(path, "rt");

	if (!fp) {
		this->unload();
		return FALSE;
	}

	m_entry_count = this->count_entries(fp);
	if (m_entry_count <= 0) {
		fclose(fp);
		this->unload();
		return FALSE;
	}

	m_entries = new list_entry[m_entry_count];
	m_entry_list = (list_entry **)malloc(sizeof(list_entry *) * m_entry_count);
	if (m_entries == NULL || m_entry_list == NULL) {
		fclose(fp);
		this->unload();
		return FALSE;
	}

	this->clear_entry_list();

	if (this->read_entries(fp) == FALSE) {
		fclose(fp);
		this->unload();
		return FALSE;
	}

	return TRUE;
}

int list::count_entries(FILE *fp) {
	int count = 0;
	char *line_buffer = (char *)malloc(LINE_BUFFER_SIZE);
	if (line_buffer == NULL)
		return -1;

	fseek(fp, 0, SEEK_SET);
	char *newline = NULL;
	while (fgets(line_buffer, LINE_BUFFER_SIZE, fp) != NULL) {
		line_buffer[LINE_BUFFER_SIZE - 1] = '\0';
		newline = strchr(line_buffer, '\n');
		if (newline != NULL)
			*newline = '\0';

		if (line_buffer[0] == '#' || string::is_empty(line_buffer))
			continue;

		++count;
	}

	free(line_buffer);

	return count;
}

bool_t list::read_entries(FILE *fp) {
	int count = 0;
	char *line_buffer = (char *)malloc(LINE_BUFFER_SIZE);
	if (line_buffer == NULL)
		return FALSE;

	fseek(fp, 0, SEEK_SET);
	char *newline = NULL;
	while (fgets(line_buffer, LINE_BUFFER_SIZE, fp) != NULL) {
		line_buffer[LINE_BUFFER_SIZE - 1] = NULL;
		newline = strchr(line_buffer, '\n');
		if (newline != NULL)
			*newline = '\0';

		if (line_buffer[0] == '#' || string::is_empty(line_buffer))
			continue;

		if (m_entries[count].parse(line_buffer) == FALSE) {
			free(line_buffer);
			return FALSE;
		}
		m_entries[count].set_index(count);

		++count;
	}

	free(line_buffer);

	return TRUE;
}

list_entry *list::get_entry(int index) {
	if (m_entries == NULL || index >= m_entry_count)
		return NULL;

	return &(m_entries[index]);
}

list_entry *list::get_list() {
	return m_entries;
}

list_entry **list::filter(const char *search, int *filtered_count) {
	*filtered_count = 0;
	if (m_entry_count == 0)
		return NULL;

	this->clear_entry_list();

	list_entry **pptr = m_entry_list;

	for (int i = 0; i < m_entry_count; ++i) {
		if (search != NULL && strlen(search) > 0 &&
			!m_entries[i].match(search))
			continue;

		*pptr = &(m_entries[i]);
		++pptr;
		++(*filtered_count);
	}

	*pptr = NULL;

	return m_entry_list;
}

void list::clear_entry_list() {
	memset(m_entry_list, 0x00, sizeof(list_entry *) * m_entry_count);
}

void list::unload() {
	if (m_entries != NULL)
		delete m_entries;

	if (m_entry_list != NULL)
		free(m_entry_list);

	m_entries = NULL;
	m_entry_list = NULL;
	m_entry_count = 0;
}